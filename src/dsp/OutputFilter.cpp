#include "OutputFilter.h"
#include "ResonantFilter.h"

namespace
{
const juce::ParameterID toneTag { "out_tone", VersionHints::original };
const juce::ParameterID levelTag { "out_level", VersionHints::original };

constexpr auto centreFreq = 800.0f;
const auto freqRange = chowdsp::ParamUtils::createNormalisableRange (300.0f, 7000.0f, centreFreq);
} // namespace

OutputFilter::OutputFilter (AudioProcessorValueTreeState& vts)
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (toneParam, vts, toneTag);
    loadParameterPointer (levelDBParam, vts, levelTag);
    loadParameterPointer (bounceParam, vts, ResTags::bounceTag);
}

void OutputFilter::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createFreqParameter (params, toneTag, "Tone", freqRange.start, freqRange.end, centreFreq, centreFreq);
    createGainDBParameter (params, levelTag, "Level", -30.0f, 30.0f, 0.0f);
}

float OutputFilter::getGainFromParam() const
{
    const auto toneMakeupDB = (freqRange.convertTo0to1 (toneParam->getCurrentValue()) - 0.5f) * -6.0f;
    const auto bounceMakeupDB = 14.0f * std::pow (bounceParam->getCurrentValue(), 2.5f);
    return Decibels::decibelsToGain (levelDBParam->getCurrentValue() + bounceMakeupDB + toneMakeupDB + 3.5f);
}

void OutputFilter::reset (double sampleRate)
{
    fs = (float) sampleRate;
    std::fill (z, &z[2], 0.0f);

    freqSmooth.reset (sampleRate, 0.05);
    gainSmooth.reset (sampleRate, 0.05);

    freqSmooth.setCurrentAndTargetValue (toneParam->getCurrentValue());
    gainSmooth.setCurrentAndTargetValue (getGainFromParam());

    calcCoefs (freqSmooth.getTargetValue(), gainSmooth.getTargetValue());
}

void OutputFilter::calcCoefs (float freq, float gain)
{
    float wc = MathConstants<float>::twoPi * freq / fs;
    float c = 1.0f / dsp::FastMathApproximations::tan (wc / 2.0f);
    float a0 = c + 1.0f;

    b[0] = gain / a0;
    b[1] = b[0];
    a[1] = (1.0f - c) / a0;
}

void OutputFilter::processBlock (float* buffer, const int numSamples)
{
    freqSmooth.setTargetValue (toneParam->getCurrentValue());
    gainSmooth.setTargetValue (getGainFromParam());

    if (freqSmooth.isSmoothing() || gainSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            calcCoefs (freqSmooth.getNextValue(), gainSmooth.getNextValue());
            buffer[n] = processSample (buffer[n]);
        }

        return;
    }

    for (int n = 0; n < numSamples; ++n)
        buffer[n] = processSample (buffer[n]);
}
