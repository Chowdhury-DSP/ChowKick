#pragma once

#include <pch.h>

namespace NoiseTags
{
const String amtTag = "noise_amt";
const String decayTag = "noise_decay";
const String freqTag = "noise_freq";
const String typeTag = "noise_type";
}; // namespace NoiseTags

class Noise
{
public:
    Noise (AudioProcessorValueTreeState& vts);

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (chowdsp::AudioBlock<Vec>& block, int numSamples);

private:
    std::atomic<float>* amtParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* freqParam = nullptr;
    std::atomic<float>* typeParam = nullptr;

    using NoiseType = chowdsp::Noise<Vec>;
    NoiseType noise;

    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> decaySmooth;

    HeapBlock<char> noiseData;
    chowdsp::AudioBlock<Vec> noiseBuffer;

    chowdsp::StateVariableFilter<Vec, chowdsp::StateVariableFilterType::Lowpass> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Noise)
};
