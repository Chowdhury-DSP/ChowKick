#pragma once

#include <pch.h>

namespace NoiseTags
{
const String amtTag = "noise_amt";
const String freqTag = "noise_freq";
const String typeTag = "noise_type";
}; // namespace NoiseTags

class Noise
{
public:
    Noise (AudioProcessorValueTreeState& vts);

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (dsp::AudioBlock<Vec>& block, int numSamples);

private:
    std::atomic<float>* amtParam = nullptr;
    std::atomic<float>* freqParam = nullptr;
    std::atomic<float>* typeParam = nullptr;

    using NoiseType = chowdsp::Noise<float>;
    NoiseType noise;

    AudioBuffer<float> buffer;

    chowdsp::StateVariableFilter<float> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Noise)
};
