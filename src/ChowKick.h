#pragma once

#include <JuceHeader.h>

class ChowKick : public chowdsp::PluginBase<ChowKick>
{
public:
    ChowKick() {}

    static void addParameters (Parameters& params);
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processAudioBlock (AudioBuffer<float>& buffer) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChowKick)
};
