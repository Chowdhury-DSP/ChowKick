#pragma once

#include "dsp/ResonantFilter.h"

class FilterViewHelper
{
public:
    FilterViewHelper (const ResonantFilter& filt) : filter (filt)
    {
    }

    static inline float drive_deriv (float x)
    {
        return 1.0f - std::pow (std::tanh (x), 2.0f);
    }

    float getResFrequency() const noexcept { return freqVal; }

    void prepare()
    {
        freqVal = filter.linkParam->get() ? 100.0f : filter.freqParam->get();
        gVal = filter.getGVal();
        qVal = filter.qParam->get();
        T = 1.0f / filter.fs;

        b1 = filter.b[1].get (0);
        b2 = filter.b[2].get (0);
        a1 = filter.a[1].get (0);
        a2 = filter.a[2].get (0);

        std::tie (b0Corr, b1Corr, b2Corr, a0Corr, a1Corr, a2Corr) = filter.getNLCorrections (b1, b2, a1, a2, T);
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
        auto b0Coeff = 1.0f + b0Corr;
        auto b1Coeff = (s / qVal) + s * b1Corr;
        auto b2Coeff = (s * s) + (s * s * b2Corr);

        auto a0Coeff = (gVal + 1.0f) + a0Corr;
        auto a1Coeff = (gVal * s / qVal) + s * a1Corr;
        auto a2Coeff = (s * s * (gVal + 1.0f)) + (s * s * a2Corr);

        return std::abs ((b2Coeff + b1Coeff + b0Coeff) / (a2Coeff + a1Coeff + a0Coeff));
    }

private:
    const ResonantFilter& filter;

    float freqVal, qVal, gVal;
    float T;
    float b0Corr, b1Corr, b2Corr;
    float a0Corr, a1Corr, a2Corr;
    float b1, b2, a1, a2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterViewHelper)
};
