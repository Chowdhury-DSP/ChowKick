#pragma once

#include "Trigger.h"
#include "ResonantFilterProcs.h"

namespace ResTags
{
const String freqTag = "res_freq";
const String linkTag = "res_link";
const String qTag = "res_q";
const String dampTag = "res_damp";
const String tightTag = "res_tight";
const String bounceTag = "res_bounce";
const String modeTag = "res_mode";
} // namespace ResTags

class ResonantFilter
{
public:
    ResonantFilter (AudioProcessorValueTreeState& vts, const Trigger& trigger);

    static void addParameters (Parameters& params);
    void reset (double sampleRate);
    void calcCoefs (Vec freq, float Q, float G);
    void processBlock (dsp::AudioBlock<Vec>& block, const int numSamples);

    void setFreqMult (float newMult) { freqMult = newMult; }
    Vec getFrequencyHz() const noexcept;
    float getGVal() const noexcept;
    float getD1Val() const noexcept;
    float getD2Val() const noexcept;
    float getD3Val() const noexcept;

    auto getNLCorrections (float b1, float b2, float a1, float a2, float T) const
    {
        auto curMode = static_cast<int> (modeParam->load());
        switch (curMode)
        {
        case 0: // Basic
            return baseProc.getNLFilterCorrections (*tightParam, *bounceParam, b1, b2, a1, a2, T);
        default:
            return std::make_tuple (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        };
    }

    friend class FilterViewHelper;

private:
    template <typename ProcType>
    void processBlockInternal (dsp::AudioBlock<Vec>& block, const int numSamples, const ProcType& proc);

    const Trigger& trigger;

    std::atomic<float>* freqParam = nullptr;
    std::atomic<float>* linkParam = nullptr;
    std::atomic<float>* qParam = nullptr;
    std::atomic<float>* dampParam = nullptr;
    std::atomic<float>* tightParam = nullptr;
    std::atomic<float>* bounceParam = nullptr;
    std::atomic<float>* modeParam = nullptr;
    float freqMult = 1.0f;

    using FreqSmooth = chowdsp::SIMDUtils::SIMDSmoothedValue<Vec::ElementType, ValueSmoothingTypes::Multiplicative>;
    FreqSmooth freqSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> qSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> gSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d1Smooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d2Smooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d3Smooth;

    float fs = 44100.0f;
    Vec a[3] = { 1.0f, 0.0f, 0.0f };
    Vec b[3] = { 1.0f, 0.0f, 0.0f };
    Vec z[3];

    BasicFilterProc baseProc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonantFilter)
};
