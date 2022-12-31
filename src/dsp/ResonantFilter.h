#pragma once

#include "ResonantFilterProcs.h"
#include "Trigger.h"

namespace ResTags
{
const juce::ParameterID freqTag { "res_freq", VersionHints::original };
const juce::ParameterID linkTag { "res_link", VersionHints::original };
const juce::ParameterID qTag { "res_q", VersionHints::original };
const juce::ParameterID dampTag { "res_damp", VersionHints::original };
const juce::ParameterID tightTag { "res_tight", VersionHints::original };
const juce::ParameterID bounceTag { "res_bounce", VersionHints::original };
const juce::ParameterID modeTag { "res_mode", VersionHints::original };
const juce::ParameterID portamentoTag { "res_portamento", VersionHints::original };
} // namespace ResTags

class ResonantFilter
{
public:
    ResonantFilter (AudioProcessorValueTreeState& vts, const Trigger& trigger);

    static void addParameters (Parameters& params);
    void reset (double sampleRate);
    void calcCoefs (Vec freq, float Q, float G);
    void processBlock (chowdsp::AudioBlock<Vec>& block, int numSamples);

    void setFreqMult (float newMult) { freqMult = newMult; }
    Vec getFrequencyHz() const noexcept;
    float getGVal() const noexcept;

    auto getNLCorrections (float b1, float b2, float a1, float a2, float T) const
    {
        auto curMode = modeParam->getIndex();
        switch (curMode)
        {
            case 0: // Linear
                return linProc.getNLFilterCorrections (*tightParam, *bounceParam, b1, b2, a1, a2, T);
            case 1: // Basic
                return baseProc.getNLFilterCorrections (*tightParam, *bounceParam, b1, b2, a1, a2, T);
            case 2: // Bouncy
                return bouncyProc.getNLFilterCorrections (*tightParam, *bounceParam, b1, b2, a1, a2, T);
            default:
                return std::make_tuple (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        };
    }

    friend class FilterViewHelper;

private:
    template <typename ProcType>
    void processBlockInternal (chowdsp::AudioBlock<Vec>& block, int numSamples, const ProcType& proc);

    const Trigger& trigger;

    chowdsp::FloatParameter* freqParam = nullptr;
    chowdsp::BoolParameter* linkParam = nullptr;
    chowdsp::FloatParameter* qParam = nullptr;
    chowdsp::FloatParameter* dampParam = nullptr;
    chowdsp::FloatParameter* tightParam = nullptr;
    chowdsp::FloatParameter* bounceParam = nullptr;
    chowdsp::ChoiceParameter* modeParam = nullptr;
    chowdsp::FloatParameter* portamentoParam = nullptr;
    float freqMult = 1.0f;

    using FreqSmooth = chowdsp::SIMDUtils::SIMDSmoothedValue<chowdsp::SampleTypeHelpers::NumericType<Vec>, ValueSmoothingTypes::Multiplicative>;
    FreqSmooth freqSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> qSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> gSmooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d1Smooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d2Smooth;
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> d3Smooth;

    float fs = 44100.0f;
    Vec a[3] = { 1.0f, 0.0f, 0.0f };
    Vec b[3] = { 1.0f, 0.0f, 0.0f };
    Vec z[3] {};

    float prevPortamento = 0.0f;

    LinearFilterProc linProc;
    BasicFilterProc baseProc;
    BouncyFilterProc bouncyProc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonantFilter)
};
