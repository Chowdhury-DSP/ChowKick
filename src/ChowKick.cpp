#include "ChowKick.h"
#include "gui/CustomLNFs.h"
#include "gui/DisabledSlider.h"
#include "gui/FilterViewer.h"
#include "gui/PulseViewer.h"
#include "gui/TuningMenu.h"
#include "presets/PresetComp.h"

#if JUCE_IOS
#include "gui/TipJar.h"
#endif

ChowKick::ChowKick() : trigger (vts),
                       resFilter (vts, trigger),
                       outFilter (vts)
{
    scope = magicState.createAndAddObject<foleys::MagicOscilloscope> ("scope");
}

void ChowKick::addParameters (Parameters& params)
{
    Trigger::addParameters (params);
    PulseShaper::addParameters (params);
    ResonantFilter::addParameters (params);
    OutputFilter::addParameters (params);
    params.push_back (std::make_unique<AudioParameterInt> ("preset", "Preset", 0, PresetManager::maxNumPresets(), 0));
}

void ChowKick::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    monoBuffer.setSize (1, samplesPerBlock);
    fourVoiceBuffer = dsp::AudioBlock<Vec> (fourVoiceData, 1, (size_t) samplesPerBlock);

    pulseShaper = std::make_unique<PulseShaper> (vts, sampleRate);
    resFilter.reset (sampleRate);
    outFilter.reset (sampleRate);

    dcBlocker.prepare ({ sampleRate, (uint32) samplesPerBlock, 1 });
    dcBlocker.setCutoffFrequency (10.0f);

    scope->prepareToPlay (sampleRate, samplesPerBlock);
}

void ChowKick::releaseResources()
{
}

void reduceBlock (const dsp::AudioBlock<Vec>& block4, AudioBuffer<float>& buffer)
{
    auto* x = block4.getChannelPointer (0);
    auto* y = buffer.getWritePointer (0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        y[i] = x[i].sum();
}

void ChowKick::processSynth (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();

    // set up buffers
    buffer.clear();
    monoBuffer.setSize (1, numSamples, false, false, true);
    monoBuffer.clear();
    fourVoiceBuffer.clear();

    magicState.processMidiBuffer (midi, numSamples);

    trigger.processBlock (fourVoiceBuffer, numSamples, midi);
    pulseShaper->processBlock (fourVoiceBuffer, numSamples);
    resFilter.processBlock (fourVoiceBuffer, numSamples);
    reduceBlock (fourVoiceBuffer, monoBuffer);
    outFilter.processBlock (monoBuffer.getWritePointer (0), numSamples);

    dsp::AudioBlock<float> monoBlock (monoBuffer);
    dsp::ProcessContextReplacing<float> monoContext (monoBlock);
    dcBlocker.process<dsp::ProcessContextReplacing<float>, chowdsp::StateVariableFilterType::Highpass> (monoContext);

    // copy monoBuffer to other channels
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, monoBuffer.getReadPointer (0), numSamples);

    scope->pushSamples (buffer);
}

AudioProcessorEditor* ChowKick::createEditor()
{
    auto builder = chowdsp::createGUIBuilder (magicState);
    builder->registerFactory ("PulseViewer", &PulseViewerItem::factory);
    builder->registerFactory ("FilterViewer", &FilterViewerItem::factory);
    builder->registerFactory ("DisabledSlider", &DisabledSlider::factory);
    builder->registerFactory ("PresetComp", &PresetComponentItem::factory);
    builder->registerFactory ("TuningMenu", &TuningMenuItem::factory);
    builder->registerLookAndFeel ("SliderLNF", std::make_unique<SliderLNF>());
    builder->registerLookAndFeel ("BottomBarLNF", std::make_unique<BottomBarLNF>());
    builder->registerLookAndFeel ("ComboBoxLNF", std::make_unique<ComboBoxLNF>());

#if JUCE_IOS
    builder->registerFactory ("TipJar", &TipJarItem::factory);
    auto editor = new foleys::MagicPluginEditor (magicState, BinaryData::gui_ios_xml, BinaryData::gui_ios_xmlSize, std::move (builder));
#else
    auto editor = new foleys::MagicPluginEditor (magicState, BinaryData::gui_xml, BinaryData::gui_xmlSize, std::move (builder));
#endif

    // we need to set resize limits for StandalonePluginHolder
    editor->setResizeLimits (10, 10, 2000, 2000);

    return editor;
}

//==============================================================================
int ChowKick::getNumPrograms()
{
    return presetManager.getNumPresets();
}

int ChowKick::getCurrentProgram()
{
    return (int) *vts.getRawParameterValue ("preset");
}

void ChowKick::setCurrentProgram (int index)
{
    if (index > presetManager.maxNumPresets())
        return;

    auto& presetParam = *vts.getRawParameterValue ("preset");
    if ((int) presetParam == index)
        return;

    if (presetManager.setPreset (vts, index))
    {
        presetParam = (float) index;
        presetManager.presetUpdated();
        updateHostDisplay (AudioProcessorListener::ChangeDetails().withProgramChanged (true));
    }
}

const String ChowKick::getProgramName (int index)
{
    return presetManager.getPresetName (index);
}

void ChowKick::getStateInformation (MemoryBlock& destData)
{
    auto state = vts.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());

    auto tuningXml = std::make_unique<XmlElement> ("tuning_data");
    trigger.getTuningState (tuningXml.get());
    xml->addChildElement (tuningXml.release());

    copyXmlToBinary (*xml, destData);
}

void ChowKick::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (vts.state.getType()))
        {
            if (auto* tuningXml = xmlState->getChildByName ("tuning_data"))
                trigger.setTuningState (tuningXml);
            else
                trigger.resetTuning();

            vts.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }

    presetManager.presetUpdated();
}

// This creates new instances of the plugin
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChowKick();
}
