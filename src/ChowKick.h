#pragma once

#include "dsp/OutputFilter.h"
#include "dsp/PulseShaper.h"
#include "dsp/ResonantFilter.h"
#include "dsp/Trigger.h"
#include "presets/PresetManager.h"

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

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    PresetManager& getPresetManager() { return presetManager; }

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    AudioBuffer<float> monoBuffer;

    HeapBlock<char> fourVoiceData;
    dsp::AudioBlock<Vec> fourVoiceBuffer;
    static_assert (Vec::size() == 4, "SIMD width must equal 4!");

    Trigger trigger;
    std::unique_ptr<PulseShaper> pulseShaper;
    ResonantFilter resFilter;
    OutputFilter outFilter;
    chowdsp::StateVariableFilter<float> dcBlocker;

    foleys::MagicPlotSource* scope = nullptr;
    PresetManager presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChowKick)
};
