#pragma once

#include <pch.h>

namespace NoiseTags
{
const juce::ParameterID amtTag { "noise_amt", VersionHints::original };
const juce::ParameterID decayTag { "noise_decay", VersionHints::original };
const juce::ParameterID freqTag { "noise_freq", VersionHints::original };
const juce::ParameterID typeTag { "noise_type", VersionHints::original };
} // namespace NoiseTags

class Noise
{
public:
    explicit Noise (AudioProcessorValueTreeState& vts);

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (chowdsp::AudioBlock<Vec>& block, int numSamples);

private:
    using NoiseType = chowdsp::Noise<Vec>;
    using NoiseMode = NoiseType::NoiseType;

    chowdsp::FloatParameter* amtParam = nullptr;
    chowdsp::FloatParameter* decayParam = nullptr;
    chowdsp::FloatParameter* freqParam = nullptr;
    chowdsp::EnumChoiceParameter<NoiseMode>* typeParam = nullptr;

    NoiseType noise;

    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> decaySmooth;

    HeapBlock<char> noiseData;
    chowdsp::AudioBlock<Vec> noiseBuffer;

    chowdsp::StateVariableFilter<Vec, chowdsp::StateVariableFilterType::Lowpass> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Noise)
};
