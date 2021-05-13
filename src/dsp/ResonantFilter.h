#pragma once

#include "Trigger.h"

namespace ResTags
{
    const String freqTag = "res_freq";
    const String linkTag = "res_link";
    const String qTag = "res_q";
    const String dampTag = "res_damp";
    const String tightTag = "res_tight";
    const String bounceTag = "res_bounce";
}

class ResonantFilter
{
public:
    ResonantFilter (AudioProcessorValueTreeState& vts, const Trigger& trigger);

    static void addParameters (Parameters& params);
    void reset (double sampleRate);
    void calcCoefs (float freq, float Q, float G);
    void processBlock (dsp::AudioBlock<Vec>& block, const int numSamples);

    void setFreqMult (float newMult) { freqMult = newMult; }
    float getFrequencyHz() const noexcept;
    float getGVal() const noexcept;
    float getD1Val() const noexcept;
    float getD2Val() const noexcept;
    float getD3Val() const noexcept;

    inline Vec processSample (Vec x, float d1, float d2, float d3) noexcept
    {
        auto y = z[1] + x * b[0];
        auto yDrive = drive (y, d3);
        z[1] = drive (z[2] + x * b[1] - yDrive * a[1], d1);
        z[2] = drive (x * b[2] - yDrive * a[2], d2);
        return y;
    }

    inline Vec drive (Vec x, float drive) const noexcept
    {
        using namespace chowdsp::SIMDUtils;
        return tanhSIMD (x * drive) / drive;
    }

    friend class FilterViewHelper;

private:
    const Trigger& trigger;

    std::atomic<float>* freqParam = nullptr;
    std::atomic<float>* linkParam = nullptr;
    std::atomic<float>* qParam    = nullptr;
    std::atomic<float>* dampParam = nullptr;
    std::atomic<float>* tightParam = nullptr;
    std::atomic<float>* bounceParam = nullptr;
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
    Vec z[3];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonantFilter)
};
