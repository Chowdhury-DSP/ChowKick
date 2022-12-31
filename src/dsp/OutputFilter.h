#pragma once

#include <pch.h>

class OutputFilter
{
public:
    explicit OutputFilter (AudioProcessorValueTreeState& vts);

    static void addParameters (Parameters& params);
    void reset (double sampleRate);
    void calcCoefs (float freq, float gain);
    void processBlock (float* buffer, int numSamples);
    float getGainFromParam() const;

    inline float processSample (float x) noexcept
    {
        auto y = z[1] + x * b[0];
        z[1] = x * b[1] - y * a[1];
        return y;
    }

private:
    chowdsp::FloatParameter* toneParam = nullptr;
    chowdsp::FloatParameter* levelDBParam = nullptr;
    chowdsp::FloatParameter* bounceParam = nullptr;

    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> freqSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> gainSmooth;

    float fs = 44100.0f;
    float a[2] = { 1.0f, 0.0f };
    float b[2] = { 1.0f, 0.0f };
    float z[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputFilter)
};
