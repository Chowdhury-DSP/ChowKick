#include "ResonantFilter.h"

using namespace ResTags;

ResonantFilter::ResonantFilter (AudioProcessorValueTreeState& vts, const Trigger& trig) : trigger (trig)
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (freqParam, vts, freqTag);
    loadParameterPointer (linkParam, vts, linkTag);
    loadParameterPointer (qParam, vts, qTag);
    loadParameterPointer (dampParam, vts, dampTag);
    loadParameterPointer (tightParam, vts, tightTag);
    loadParameterPointer (bounceParam, vts, bounceTag);
    loadParameterPointer (modeParam, vts, modeTag);
    loadParameterPointer (portamentoParam, vts, portamentoTag);
}

void ResonantFilter::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;

    createFreqParameter (params, freqTag, "Frequency", 30.0f, 500.0f, 100.0f, 80.0f);

    const auto qRange = createNormalisableRange (0.1f, 2.0f, chowdsp::CoefficientCalculators::butterworthQ<float>);
    emplace_param<chowdsp::FloatParameter> (params, qTag, "Q", qRange, 0.5f, &floatValToString, &stringToFloatVal);

    createPercentParameter (params, dampTag, "Damping", 0.5f);
    createPercentParameter (params, tightTag, "Tight", 0.5f);
    createPercentParameter (params, bounceTag, "Bounce", 0.0f);

    emplace_param<chowdsp::BoolParameter> (params, linkTag, "Link", false);
    emplace_param<chowdsp::ChoiceParameter> (params, modeTag, "Res. Mode", StringArray { "Linear", "Basic", "Bouncy" }, 1);

    createTimeMsParameter (params, portamentoTag, "Portamento", createNormalisableRange (0.1f, 200.0f, 50.0f), 50.0f);
}

void ResonantFilter::reset (double sampleRate)
{
    fs = (float) sampleRate;
    std::fill (z, &z[3], 0.0f);

    prevPortamento = *portamentoParam;
    freqSmooth.reset (sampleRate, prevPortamento * 0.001f);
    qSmooth.reset (sampleRate, 0.05f);
    gSmooth.reset (sampleRate, 0.05f);
    d1Smooth.reset (sampleRate, 0.05f);
    d2Smooth.reset (sampleRate, 0.05f);
    d3Smooth.reset (sampleRate, 0.05f);

    freqSmooth.setCurrentAndTargetValue (getFrequencyHz());
    qSmooth.setCurrentAndTargetValue (*qParam);
    gSmooth.setCurrentAndTargetValue (getGVal());

    auto curMode = modeParam->getIndex();
    float d1 = 0.0f, d2 = 0.0f, d3 = 0.0f; // NOLINT
    switch (curMode)
    {
        case 0: // Linear
            std::tie (d1, d2, d3) = linProc.getDriveValues (tightParam->getCurrentValue(), bounceParam->getCurrentValue());
            break;
        case 1: // Basic
            std::tie (d1, d2, d3) = baseProc.getDriveValues (tightParam->getCurrentValue(), bounceParam->getCurrentValue());
            break;
        case 2: // Bouncy
            std::tie (d1, d2, d3) = bouncyProc.getDriveValues (tightParam->getCurrentValue(), bounceParam->getCurrentValue());
            break;
        default:
            break;
    };
    d1Smooth.setCurrentAndTargetValue (d1);
    d2Smooth.setCurrentAndTargetValue (d2);
    d3Smooth.setCurrentAndTargetValue (d3);

    calcCoefs (freqSmooth.getTargetValue(), qSmooth.getTargetValue(), gSmooth.getTargetValue());
}

Vec ResonantFilter::getFrequencyHz() const noexcept
{
    return (Vec) freqMult * (linkParam->get() ? trigger.getFrequencyHz() : (Vec) freqParam->getCurrentValue());
}

float ResonantFilter::getGVal() const noexcept
{
    constexpr float lowDamp = 0.0001f;
    constexpr float highDamp = 0.5f;

    return lowDamp * std::pow ((highDamp / lowDamp), dampParam->getCurrentValue());
}

void ResonantFilter::calcCoefs (Vec freq, float Q, float G)
{
    const auto wc = freq * MathConstants<float>::twoPi / fs;
    const auto wS = xsimd::sin (wc);
    const auto wC = xsimd::cos (wc);
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
void ResonantFilter::processBlockInternal (chowdsp::AudioBlock<Vec>& block, const int numSamples, const ProcType& proc)
{
    auto [d1, d2, d3] = proc.getDriveValues (tightParam->getCurrentValue(), bounceParam->getCurrentValue());
    d1Smooth.setTargetValue (d1);
    d2Smooth.setTargetValue (d2);
    d3Smooth.setTargetValue (d3);

    auto* x = block.getChannelPointer (0);
    if (freqSmooth.isSmoothing() || qSmooth.isSmoothing() || gSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            calcCoefs (freqSmooth.getNextValue(), qSmooth.getNextValue(), gSmooth.getNextValue());
            x[n] = proc (x[n], b, a, z, d1Smooth.getNextValue(), d2Smooth.getNextValue(), d3Smooth.getNextValue());
        }

        return;
    }

    if (d1Smooth.isSmoothing() || d2Smooth.isSmoothing() || d3Smooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
            x[n] = proc (x[n], b, a, z, d1Smooth.getNextValue(), d2Smooth.getNextValue(), d3Smooth.getNextValue());

        return;
    }

    for (int n = 0; n < numSamples; ++n)
        x[n] = proc (x[n], b, a, z, d1, d2, d3);
}

void ResonantFilter::processBlock (chowdsp::AudioBlock<Vec>& block, const int numSamples)
{
    if (*portamentoParam != prevPortamento)
    {
        prevPortamento = *portamentoParam;
        freqSmooth.reset (fs, prevPortamento * 0.001f);
    }

    freqSmooth.setTargetValue (getFrequencyHz());
    qSmooth.setTargetValue (*qParam);
    gSmooth.setTargetValue (getGVal());

    auto curMode = modeParam->getIndex();
    switch (curMode)
    {
        case 0: // Linear
            processBlockInternal (block, numSamples, linProc);
            break;
        case 1: // Basic
            processBlockInternal (block, numSamples, baseProc);
            break;
        case 2: // Bouncy
            processBlockInternal (block, numSamples, bouncyProc);
            break;
        default:
            break;
    }
}
