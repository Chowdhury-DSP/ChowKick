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

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    auto& getVTS() { return vts; }
    auto& getTrigger() { return trigger; }
    auto* getOpenGLHelper() { return openGLHelper.get(); }
    auto* getHostContextProvider() { return hostContextProvider.get(); }

private:
    chowdsp::PluginLogger logger;
    chowdsp::SharedPluginSettings pluginSettings;

    AudioBuffer<float> monoBuffer;

    HeapBlock<char> fourVoiceData;
    chowdsp::AudioBlock<Vec> fourVoiceBuffer;
    static_assert (Vec::size == 4, "SIMD width must equal 4!");

    Trigger trigger;
    Noise noise;
    std::optional<PulseShaper> pulseShaper;
    ResonantFilter resFilter;
    OutputFilter outFilter;
    chowdsp::StateVariableFilter<float, chowdsp::StateVariableFilterType::Highpass> dcBlocker;

    foleys::MagicPlotSource* scope = nullptr;
    std::unique_ptr<chowdsp::HostContextProvider> hostContextProvider;
    std::unique_ptr<chowdsp::OpenGLHelper> openGLHelper = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChowKick)
};
