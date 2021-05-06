#include "ChowKick.h"

ChowKick::ChowKick() :
    trigger (vts),
    resFilter (vts, trigger),
    outFilter (vts)
{
}

void ChowKick::addParameters (Parameters& params)
{
    Trigger::addParameters (params);
    PulseShaper::addParameters (params);
    ResonantFilter::addParameters (params);
    OutputFilter::addParameters (params);
}

void ChowKick::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    monoBuffer.setSize (1, samplesPerBlock);
    pulseShaper = std::make_unique<PulseShaper> (vts, sampleRate);
    resFilter.reset (sampleRate);
    outFilter.reset (sampleRate);
}

void ChowKick::releaseResources()
{
}

void ChowKick::processSynth (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();

    // set up buffers
    buffer.clear();
    monoBuffer.setSize (1, numSamples, false, false, true);
    monoBuffer.clear();

    magicState.processMidiBuffer (midi, numSamples);

    trigger.processBlock (monoBuffer, midi);
    pulseShaper->processBlock (monoBuffer.getWritePointer (0), numSamples);
    resFilter.processBlock (monoBuffer.getWritePointer (0), numSamples);
    outFilter.processBlock (monoBuffer.getWritePointer (0), numSamples);

    // copy monoBuffer to other channels
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, monoBuffer.getReadPointer (0), numSamples);
}

AudioProcessorEditor* ChowKick::createEditor()
{
    auto editor = new foleys::MagicPluginEditor (magicState, BinaryData::gui_xml, BinaryData::gui_xmlSize);
    editor->setResizeLimits (10, 10, 1000, 1000);

    return editor;
}

// This creates new instances of the plugin
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChowKick();
}
