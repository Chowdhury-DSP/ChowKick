#include "ResonantFilter.h"

using namespace ResTags;

ResonantFilter::ResonantFilter (AudioProcessorValueTreeState& vts, const Trigger& trigger) :
    trigger (trigger)
{
    freqParam = vts.getRawParameterValue (freqTag);
    linkParam = vts.getRawParameterValue (linkTag);
    qParam    = vts.getRawParameterValue (qTag);
    gParam    = vts.getRawParameterValue (gTag);
    drive1Param = vts.getRawParameterValue (d1Tag);
    drive2Param = vts.getRawParameterValue (d2Tag);
    drive3Param = vts.getRawParameterValue (d3Tag);
}

void ResonantFilter::addParameters (Parameters& params)
{
    NormalisableRange<float> freqRange (30.0f, 500.0f);
    freqRange.setSkewForCentre (100.0f);
    float freqDefault = 80.0f;

    NormalisableRange<float> qRange (0.1f, 2.0f);
    qRange.setSkewForCentre (1.0f / MathConstants<float>::sqrt2);

    NormalisableRange<float> gRange (0.0001f, 0.5f);
    gRange.setSkewForCentre (0.01f);

    NormalisableRange<float> driveRange (0.1f, 5.0f);
    driveRange.setSkewForCentre (1.0f);

    params.push_back (std::make_unique<AudioParameterFloat> (freqTag, "Freq", freqRange, freqDefault));
    params.push_back (std::make_unique<AudioParameterBool>  (linkTag, "Link", false));
    params.push_back (std::make_unique<AudioParameterFloat> (qTag, "Q", qRange, 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (gTag, "G", gRange, 0.01f));
    params.push_back (std::make_unique<AudioParameterFloat> (d1Tag, "Drive1", driveRange, 1.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (d2Tag, "Drive2", driveRange, 1.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (d3Tag, "Drive3", driveRange, 1.0f));
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
    gSmooth.setCurrentAndTargetValue (*gParam);
    d1Smooth.setCurrentAndTargetValue (*drive1Param);
    d2Smooth.setCurrentAndTargetValue (*drive2Param);
    d3Smooth.setCurrentAndTargetValue (*drive3Param);

    calcCoefs (freqSmooth.getTargetValue(), qSmooth.getTargetValue(), gSmooth.getTargetValue());
}

float ResonantFilter::getFrequencyHz() const noexcept
{
    return freqMult * (bool (*linkParam) ? trigger.getFrequencyHz() : *freqParam);
}

void ResonantFilter::calcCoefs (float freq, float Q, float G)
{
    const auto wc = MathConstants<float>::twoPi * freq / fs;
    const auto wS = std::sin (wc);
    const auto wC = std::cos (wc);
    const auto alpha = wS / (2.0f * Q);

    const auto a0 = (G + 1.0f) + alpha * G;
    b[0] = (1.0f + alpha) / a0;
    b[1] = -2.0f * wC / a0;
    b[2] = (1.0f - alpha) / a0;
    a[0] = 1.0f;
    a[1] = -2.0f * wC * (G + 1.0f)/ a0;
    a[2] = ((G + 1.0f) - G * alpha) / a0;
}

void ResonantFilter::processBlock (float* buffer, const int numSamples)
{
    freqSmooth.setTargetValue (getFrequencyHz());
    qSmooth.setTargetValue (*qParam);
    gSmooth.setTargetValue (*gParam);
    d1Smooth.setTargetValue (*drive1Param);
    d2Smooth.setTargetValue (*drive2Param);
    d3Smooth.setTargetValue (*drive3Param);

    if (freqSmooth.isSmoothing() || qSmooth.isSmoothing() || gSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            calcCoefs (freqSmooth.getNextValue(), qSmooth.getNextValue(), gSmooth.getNextValue());
            buffer[n] = processSample (buffer[n], d1Smooth.getNextValue(), d2Smooth.getNextValue(), d3Smooth.getNextValue());
        }

        return;
    }

    for (int n = 0; n < numSamples; ++n)
        buffer[n] = processSample (buffer[n], d1Smooth.getNextValue(), d2Smooth.getNextValue(), d3Smooth.getNextValue());
}
