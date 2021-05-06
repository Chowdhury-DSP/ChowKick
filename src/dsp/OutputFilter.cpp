#include "OutputFilter.h"

namespace
{
    const String toneTag = "out_tone";
    const String levelTag = "out_level";
}

OutputFilter::OutputFilter (AudioProcessorValueTreeState& vts)
{
    toneParam    = vts.getRawParameterValue (toneTag);
    levelDBParam = vts.getRawParameterValue (levelTag);
}

void OutputFilter::addParameters (Parameters& params)
{
    NormalisableRange<float> freqRange (300.0f, 7000.0f);
    freqRange.setSkewForCentre (800.0f);

    params.push_back (std::make_unique<AudioParameterFloat> (toneTag, "Tone", freqRange, 800.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (levelTag, "Level [dB]", -30.0f, 30.0f, 0.0f));
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
