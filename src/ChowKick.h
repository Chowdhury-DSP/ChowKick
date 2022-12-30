#pragma once

#include "dsp/Noise.h"
#include "dsp/OutputFilter.h"
#include "dsp/PulseShaper.h"
#include "dsp/ResonantFilter.h"
#include "dsp/Trigger.h"

class ChowKick : public chowdsp::SynthBase<ChowKick>
{
public:
    ChowKick();

    static void addParameters (Parameters& params);
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processSynth (AudioBuffer<float>& buffer, MidiBuffer& midi) override;

    AudioProcessorEditor* createEditor() override;
    AudioProcessorValueTreeState& getVTS() { return vts; }

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    Trigger& getTrigger() { return trigger; }

private:
    AudioBuffer<float> monoBuffer;

    HeapBlock<char> fourVoiceData;
    chowdsp::AudioBlock<Vec> fourVoiceBuffer;
    static_assert (Vec::size == 4, "SIMD width must equal 4!");

    Trigger trigger;
    Noise noise;
    std::unique_ptr<PulseShaper> pulseShaper;
    ResonantFilter resFilter;
    OutputFilter outFilter;
    chowdsp::StateVariableFilter<float, chowdsp::StateVariableFilterType::Highpass> dcBlocker;

    foleys::MagicPlotSource* scope = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChowKick)
};
