#pragma once

#include "Trigger.h"

namespace ResTags
{
    const String freqTag = "res_freq";
    const String linkTag = "res_link";
    const String qTag = "res_q";
    const String gTag = "res_g";
    const String d1Tag = "res_d1";
    const String d2Tag = "res_d2";
    const String d3Tag = "res_d3";
}

class ResonantFilter
{
public:
    ResonantFilter (AudioProcessorValueTreeState& vts, const Trigger& trigger);

    static void addParameters (Parameters& params);
    void reset (double sampleRate);
    void calcCoefs (float freq, float Q, float G);
    void processBlock (float* buffer, const int numSamples);

    void setFreqMult (float newMult) { freqMult = newMult; }
    float getFrequencyHz() const noexcept;

    inline float processSample (float x, float d1, float d2, float d3) noexcept
    {
        auto y = z[1] + x * b[0];
        auto yDrive = drive (y, d3);
        z[1] = drive (z[2] + x * b[1] - yDrive * a[1], d1);
        z[2] = drive (x * b[2] - yDrive * a[2], d2);
        return y;
    }

    inline float drive (float x, float drive) noexcept
    {
        return std::tanh (x * drive) / drive;
    }

private:
    const Trigger& trigger;

    std::atomic<float>* freqParam = nullptr;
    std::atomic<float>* linkParam = nullptr;
    std::atomic<float>* qParam    = nullptr;
    std::atomic<float>* gParam    = nullptr;
    std::atomic<float>* drive1Param = nullptr; // state1
    std::atomic<float>* drive2Param = nullptr; // state2
    std::atomic<float>* drive3Param = nullptr; // FB
    float freqMult = 1.0f;

    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> freqSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> qSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> gSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d1Smooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d2Smooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d3Smooth;

    float fs = 44100.0f;
    float a[3] = { 1.0f, 0.0f, 0.0f };
    float b[3] = { 1.0f, 0.0f, 0.0f };
    float z[3];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonantFilter)
};
