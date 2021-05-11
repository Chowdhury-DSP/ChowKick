#pragma once

#include "dsp/ResonantFilter.h"

class FilterViewHelper
{
public:
    FilterViewHelper (const ResonantFilter& filt) :
        filter (filt)
    {
    }

    static inline float drive_deriv (float x)
    {
        return 1.0f - std::pow (std::tanh (x), 2.0f);
    }

    void prepare()
    {
        freqVal = filter.freqParam->load();
        gVal = filter.getGVal();
        qVal = filter.qParam->load();

        T = 1.0f / filter.fs;
        g1 = drive_deriv (filter.getD1Val());
        g2 = drive_deriv (filter.getD2Val());
        g3 = drive_deriv (filter.getD3Val());
    }

    inline float getMagForFreq (float freq) const noexcept
    {
        std::complex<float> s (0, freq / freqVal); // s = j (w / w0)
        auto numerator = s * s + s / qVal + 1.0f;
        auto denominator = s * s * (gVal + 1.0f) + gVal * s / qVal + gVal + 1.0f;
        return std::abs (numerator / denominator);
    }

    inline float getMagForFreqNL (float freq) const noexcept
    {
        std::complex<float> s (0, freq / freqVal); // s = j (w / w0)

        // corrected for nonlinearities!
        auto b0Corr = 1.0f - filter.b[1] - filter.b[2] + g1 * filter.b[1] + g1 * g2 * filter.b[2];
        auto b1Corr = (s / qVal) + (filter.b[2] - g1 * g2 * filter.b[2]) * s * T;
        auto b2Corr = (s * s) + (s * s * T / 4.0f) * (g1 * g2 * filter.b[2] - g1 * filter.b[1] - filter.b[2] + filter.b[1]);

        auto a0Corr = (gVal + 1.0f) - filter.a[1] - filter.a[2] + g1 * g3 * filter.a[1] + g1 * g2 * g3 * filter.a[2];
        auto a1Corr = (gVal * s / qVal) + (filter.a[2] - g1 * g2 * g3 * filter.a[2]) * s * T;
        auto a2Corr = (s * s * (gVal + 1.0f)) + (s * s * T / 4.0f) * (g1 * g2 * g3 * filter.a[2] - g1 * g3 * filter.a[1] - filter.a[2] + filter.a[1]);

        auto numerator = b2Corr + b1Corr + b0Corr;
        auto denominator = a2Corr + a1Corr + a0Corr;
        return std::abs (numerator / denominator);
    }

private:
    const ResonantFilter& filter;

    float freqVal, qVal, gVal;
    float T;
    float g1, g2, g3;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterViewHelper)
};
