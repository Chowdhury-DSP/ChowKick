#include "PulseShaper.h"

namespace
{
    const String r1Tag = "pulse_r1";
    const String r2Tag = "pulse_r2";
}

PulseShaper::PulseShaper (AudioProcessorValueTreeState& vts, double sampleRate) :
    c40 (0.015e-6, sampleRate, 0.029)
{
    d53.connectToNode (&P2);

    r1Param = vts.getRawParameterValue (r1Tag);
    r2Param = vts.getRawParameterValue (r2Tag);
}

void PulseShaper::addParameters (Parameters& params)
{
    params.push_back (std::make_unique<AudioParameterFloat> (r1Tag, "R1", 10000.0f, 200000.0f, 100000.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (r2Tag, "R2", 1000.0f, 10000.0f, 4700.0f));
}

void PulseShaper::processBlock (float* block, const int numSamples)
{
    r162.setResistanceValue (*r2Param);
    r163.setResistanceValue (*r1Param);

    for (int n = 0; n < numSamples; ++n)
        block[n] = processSample (block[n]);
}
