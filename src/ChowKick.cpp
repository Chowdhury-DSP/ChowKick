#include "ChowKick.h"
#include "gui/CustomLNFs.h"
#include "gui/DisabledSlider.h"
#include "gui/FilterViewer.h"
#include "gui/PulseViewer.h"
#include "gui/TuningMenu.h"
#include "presets/PresetManager.h"

#if JUCE_IOS
#include "gui/TipJar.h"
#endif

ChowKick::ChowKick() : trigger (vts),
                       noise (vts),
                       resFilter (vts, trigger),
                       outFilter (vts)
{
    presetManager = std::make_unique<PresetManager> (vts);
    scope = magicState.createAndAddObject<foleys::MagicOscilloscope> ("scope");
}

void ChowKick::addParameters (Parameters& params)
{
    Trigger::addParameters (params);
    Noise::addParameters (params);
    PulseShaper::addParameters (params);
    ResonantFilter::addParameters (params);
    OutputFilter::addParameters (params);
}

void ChowKick::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    monoBuffer.setSize (1, samplesPerBlock);
    fourVoiceBuffer = chowdsp::AudioBlock<Vec> (fourVoiceData, 1, (size_t) samplesPerBlock);

    trigger.prepareToPlay (sampleRate, samplesPerBlock);
    noise.prepareToPlay (sampleRate, samplesPerBlock);

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

void reduceBlock (const chowdsp::AudioBlock<Vec>& block4, AudioBuffer<float>& buffer)
{
    auto* x = block4.getChannelPointer (0);
    auto* y = buffer.getWritePointer (0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        y[i] = xsimd::reduce_add (x[i]);
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
    noise.processBlock (fourVoiceBuffer, numSamples);
    reduceBlock (fourVoiceBuffer, monoBuffer);
    outFilter.processBlock (monoBuffer.getWritePointer (0), numSamples);

    dsp::AudioBlock<float> monoBlock (monoBuffer);
    dsp::ProcessContextReplacing<float> monoContext (monoBlock);
    dcBlocker.process<dsp::ProcessContextReplacing<float>> (monoContext);

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
    builder->registerFactory ("PresetComp", &chowdsp::PresetsItem<ChowKick>::factory);
    builder->registerFactory ("TuningMenu", &TuningMenuItem::factory);
    builder->registerLookAndFeel ("SliderLNF", std::make_unique<SliderLNF>());
    builder->registerLookAndFeel ("BottomBarLNF", std::make_unique<BottomBarLNF>());
    builder->registerLookAndFeel ("ComboBoxLNF", std::make_unique<ComboBoxLNF>());
    builder->registerLookAndFeel ("TuningMenuLNF", std::make_unique<TuningMenuLNF>());

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
void ChowKick::getStateInformation (MemoryBlock& destData)
{
    auto state = vts.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());

    auto tuningXml = std::make_unique<XmlElement> ("tuning_data");
    trigger.getTuningState (tuningXml.get());
    xml->addChildElement (tuningXml.release());
    xml->addChildElement (presetManager->saveXmlState().release());

    copyXmlToBinary (*xml, destData);
}

void ChowKick::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (vts.state.getType()))
        {
            presetManager->loadXmlState (xmlState->getChildByName (chowdsp::PresetManager::presetStateTag));

            if (auto* tuningXml = xmlState->getChildByName ("tuning_data"))
                trigger.setTuningState (tuningXml);
            else
                trigger.resetTuning();

            xmlState->deleteAllChildElementsWithTagName ("tuning_data");
            xmlState->deleteAllChildElementsWithTagName (chowdsp::PresetManager::presetStateTag);

            vts.replaceState (ValueTree::fromXml (*xmlState));
        }
    }
}

// This creates new instances of the plugin
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChowKick();
}
