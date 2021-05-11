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
    void processBlock (float* buffer, const int numSamples);

    void setFreqMult (float newMult) { freqMult = newMult; }
    float getFrequencyHz() const noexcept;
    float getGVal() const noexcept;
    float getD1Val() const noexcept;
    float getD2Val() const noexcept;
    float getD3Val() const noexcept;

    inline float processSample (float x, float d1, float d2, float d3) noexcept
    {
        auto y = z[1] + x * b[0];
        auto yDrive = drive (y, d3);
        z[1] = drive (z[2] + x * b[1] - yDrive * a[1], d1);
        z[2] = drive (x * b[2] - yDrive * a[2], d2);
        return y;
    }

    inline float drive (float x, float drive) const noexcept
    {
        return std::tanh (x * drive) / drive;
    }

    inline float drive_deriv (float x) const noexcept
    {
        return 1.0f - std::pow (std::tanh (x), 2.0f);
    }

    inline float getMagForFreq (float freq) const noexcept
    {
        std::complex<float> s (0, freq / freqParam->load()); // s = j (w / w0)
        const auto gVal = getGVal();
        const auto qVal = qParam->load();

        auto numerator = s * s + s / qVal + 1.0f;
        auto denominator = s * s * (gVal + 1.0f) + gVal * s / qVal + gVal + 1.0f;
        return std::abs (numerator / denominator);
    }

    inline float getMagForFreqNL (float freq) const noexcept
    {
        std::complex<float> s (0, freq / freqParam->load()); // s = j (w / w0)
        const auto gVal = getGVal();
        const auto qVal = qParam->load();

        const auto T = 1.0f / fs;
        const auto g1 = drive_deriv (getD1Val());
        const auto g2 = drive_deriv (getD2Val());
        const auto g3 = drive_deriv (getD3Val());

        auto b0Corr = 1.0f - b[1] - b[2] + g1 * b[1] + g1 * g2 * b[2];
        auto b1Corr = (s / qVal) + (b[2] - g1 * g2 * b[2]) * s * T;
        auto b2Corr = (s * s) + (s * s * T / 4.0f) * (g1 * g2 * b[2] - g1 * b[1] - b[2] + b[1]);

        auto a0Corr = (gVal + 1.0f) - a[1] - a[2] + g1 * g3 * a[1] + g1 * g2 * g3 * a[2];
        auto a1Corr = (gVal * s / qVal) + (a[2] - g1 * g2 * g3 * a[2]) * s * T;
        auto a2Corr = (s * s * (gVal + 1.0f)) + (s * s * T / 4.0f) * (g1 * g2 * g3 * a[2] - g1 * g3 * a[1] - a[2] + a[1]);

        auto numerator = b2Corr + b1Corr + b0Corr;
        auto denominator = a2Corr + a1Corr + a0Corr;
        return std::abs (numerator / denominator);
    }

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
    float z[3];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonantFilter)
};
