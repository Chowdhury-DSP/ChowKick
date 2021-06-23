#include "ResonantFilter.h"

using namespace ResTags;

ResonantFilter::ResonantFilter (AudioProcessorValueTreeState& vts, const Trigger& trig) : trigger (trig)
{
    freqParam = vts.getRawParameterValue (freqTag);
    linkParam = vts.getRawParameterValue (linkTag);
    qParam = vts.getRawParameterValue (qTag);
    dampParam = vts.getRawParameterValue (dampTag);
    tightParam = vts.getRawParameterValue (tightTag);
    bounceParam = vts.getRawParameterValue (bounceTag);
    modeParam = vts.getRawParameterValue (modeTag);
}

void ResonantFilter::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    NormalisableRange<float> percentRange (0.0f, 1.0f);

    NormalisableRange<float> freqRange (30.0f, 500.0f);
    freqRange.setSkewForCentre (100.0f);
    constexpr float freqDefault = 80.0f;
    params.push_back (std::make_unique<VTSParam> (freqTag,
                                                  "Frequency",
                                                  String(),
                                                  freqRange,
                                                  freqDefault,
                                                  &freqValToString,
                                                  &stringToFreqVal));

    NormalisableRange<float> qRange (0.1f, 2.0f);
    qRange.setSkewForCentre (1.0f / MathConstants<float>::sqrt2);
    params.push_back (std::make_unique<VTSParam> (qTag,
                                                  "Q",
                                                  String(),
                                                  qRange,
                                                  0.5f,
                                                  &floatValToString,
                                                  &stringToFloatVal));

    params.push_back (std::make_unique<VTSParam> (dampTag,
                                                  "Damping",
                                                  String(),
                                                  percentRange,
                                                  0.5f,
                                                  &percentValToString,
                                                  &stringToPercentVal));

    params.push_back (std::make_unique<VTSParam> (tightTag,
                                                  "Tight",
                                                  String(),
                                                  percentRange,
                                                  0.5f,
                                                  &percentValToString,
                                                  &stringToPercentVal));

    params.push_back (std::make_unique<VTSParam> (bounceTag,
                                                  "Bounce",
                                                  String(),
                                                  percentRange,
                                                  0.0f,
                                                  &percentValToString,
                                                  &stringToPercentVal));

    params.push_back (std::make_unique<AudioParameterBool> (linkTag, "Link", false));

    params.push_back (std::make_unique<AudioParameterChoice> (modeTag,
                                                              "Res. Mode",
                                                              StringArray { "Basic", "Dummy" },
                                                              0));
}

void ResonantFilter::reset (double sampleRate)
{
    fs = (float) sampleRate;
    std::fill (z, &z[3], 0.0f);

    freqSmooth.reset (sampleRate, 0.05f);
    qSmooth.reset (sampleRate, 0.05f);
    gSmooth.reset (sampleRate, 0.05f);
    d1Smooth.reset (sampleRate, 0.05f);
    d2Smooth.reset (sampleRate, 0.05f);
    d3Smooth.reset (sampleRate, 0.05f);

    freqSmooth.setCurrentAndTargetValue (getFrequencyHz());
    qSmooth.setCurrentAndTargetValue (*qParam);
    gSmooth.setCurrentAndTargetValue (getGVal());
    d1Smooth.setCurrentAndTargetValue (getD1Val());
    d2Smooth.setCurrentAndTargetValue (getD2Val());
    d3Smooth.setCurrentAndTargetValue (getD3Val());

    calcCoefs (freqSmooth.getTargetValue(), qSmooth.getTargetValue(), gSmooth.getTargetValue());
}

Vec ResonantFilter::getFrequencyHz() const noexcept
{
    return (Vec) freqMult * (bool (*linkParam) ? trigger.getFrequencyHz() : (Vec) freqParam->load());
}

float ResonantFilter::getGVal() const noexcept
{
    constexpr float lowDamp = 0.0001f;
    constexpr float highDamp = 0.5f;

    return lowDamp * std::pow ((highDamp / lowDamp), dampParam->load());
}

float ResonantFilter::getD1Val() const noexcept
{
    return 4.9f * std::pow (tightParam->load(), 4.0f) + 0.1f;
}

float ResonantFilter::getD2Val() const noexcept
{
    return 4.9f * std::pow (tightParam->load(), 6.0f) + 0.1f;
}

float ResonantFilter::getD3Val() const noexcept
{
    auto dp = bounceParam->load();
    return 4.75f * std::pow (dp, 3.0f) + 0.25f;
}

void ResonantFilter::calcCoefs (Vec freq, float Q, float G)
{
    using namespace chowdsp::SIMDUtils;

    const auto wc = freq * MathConstants<float>::twoPi / fs;
    const auto wS = sinSIMD (wc);
    const auto wC = cosSIMD (wc);
    const auto alpha = wS / (2.0f * Q);

    const auto a0 = (Vec) (G + 1.0f) + alpha * G;
    b[0] = (alpha + 1.0f) / a0;
    b[1] = wC * -2.0f / a0;
    b[2] = ((Vec) 1.0f - alpha) / a0;
    a[0] = 1.0f;
    a[1] = wC * -2.0f * (G + 1.0f) / a0;
    a[2] = ((Vec) (G + 1.0f) - alpha * G) / a0;
}

template <typename ProcType>
void ResonantFilter::processBlockInternal (dsp::AudioBlock<Vec>& block, const int numSamples, const ProcType& proc)
{
    auto [d1, d2, d3] = proc.getDriveValues (tightParam->load(), bounceParam->load());
    d1Smooth.setTargetValue (d1);
    d2Smooth.setTargetValue (d2);
    d3Smooth.setTargetValue (d3);

    auto* x = block.getChannelPointer (0);
    if (freqSmooth.isSmoothing() || qSmooth.isSmoothing() || gSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            calcCoefs (freqSmooth.getNextValue(), qSmooth.getNextValue(), gSmooth.getNextValue());
            x[n] = proc (x[n], a, b, z, d1Smooth.getNextValue(), d2Smooth.getNextValue(), d3Smooth.getNextValue());
        }

        return;
    }

    for (int n = 0; n < numSamples; ++n)
        x[n] = proc (x[n], a, b, z, d1Smooth.getNextValue(), d2Smooth.getNextValue(), d3Smooth.getNextValue());
}

void ResonantFilter::processBlock (dsp::AudioBlock<Vec>& block, const int numSamples)
{
    freqSmooth.setTargetValue (getFrequencyHz());
    qSmooth.setTargetValue (*qParam);
    gSmooth.setTargetValue (getGVal());

    auto curMode = static_cast<int> (modeParam->load());
    switch (curMode)
    {
    case 0: // Basic
        processBlockInternal (block, numSamples, baseProc);
        break;
    default:
        break;
    };
}
