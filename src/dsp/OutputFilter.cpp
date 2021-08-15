#include "OutputFilter.h"
#include "ResonantFilter.h"

namespace
{
const String toneTag = "out_tone";
const String levelTag = "out_level";

NormalisableRange<float> freqRange (300.0f, 7000.0f);
} // namespace

OutputFilter::OutputFilter (AudioProcessorValueTreeState& vts)
{
    toneParam = vts.getRawParameterValue (toneTag);
    levelDBParam = vts.getRawParameterValue (levelTag);
    bounceParam = vts.getRawParameterValue (ResTags::bounceTag);
}

void OutputFilter::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;

    freqRange.setSkewForCentre (800.0f);
    params.push_back (std::make_unique<VTSParam> (toneTag,
                                                  "Tone",
                                                  String(),
                                                  freqRange,
                                                  800.0f,
                                                  &freqValToString,
                                                  &stringToFreqVal));

    params.push_back (std::make_unique<VTSParam> (levelTag,
                                                  "Level",
                                                  String(),
                                                  NormalisableRange<float> { -30.0f, 30.0f },
                                                  0.0f,
                                                  &gainValToString,
                                                  &stringToGainVal));
}

float OutputFilter::getGainFromParam() const
{
    const auto toneMakeupDB = (freqRange.convertTo0to1 (toneParam->load()) - 0.5f) * -6.0f;
    const auto bounceMakeupDB = 14.0f * std::pow (bounceParam->load(), 2.5f);
    return Decibels::decibelsToGain (levelDBParam->load() + bounceMakeupDB + toneMakeupDB + 3.5f);
}

void OutputFilter::reset (double sampleRate)
{
    fs = (float) sampleRate;
    std::fill (z, &z[2], 0.0f);

    freqSmooth.reset (sampleRate, 0.05);
    gainSmooth.reset (sampleRate, 0.05);

    freqSmooth.setCurrentAndTargetValue (toneParam->load());
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
    freqSmooth.setTargetValue (toneParam->load());
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
