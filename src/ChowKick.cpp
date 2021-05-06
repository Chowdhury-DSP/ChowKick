#include "ChowKick.h"

void ChowKick::addParameters (Parameters& params)
{
}

void ChowKick::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void ChowKick::releaseResources()
{
}

void ChowKick::processAudioBlock (AudioBuffer<float>& buffer)
{
}

// This creates new instances of the plugin
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChowKick();
}
