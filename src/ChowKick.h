#pragma once

#include "dsp/Trigger.h"
#include "dsp/PulseShaper.h"
#include "dsp/ResonantFilter.h"
#include "dsp/OutputFilter.h"

class ChowKick : public chowdsp::SynthBase<ChowKick>
{
public:
    ChowKick();

    static void addParameters (Parameters& params);
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processSynth (AudioBuffer<float>& buffer, MidiBuffer& midi) override;

    AudioProcessorEditor* createEditor() override;

private:
    AudioBuffer<float> monoBuffer;

    Trigger trigger;
    std::unique_ptr<PulseShaper> pulseShaper;
    ResonantFilter resFilter;
    OutputFilter outFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChowKick)
};
