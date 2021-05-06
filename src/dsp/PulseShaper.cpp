#include "PulseShaper.h"

namespace
{
    const String r1Tag = "pulse_r1";
    const String r2Tag = "pulse_r2";
}

PulseShaper::PulseShaper (AudioProcessorValueTreeState& vts)
{
    r1Param = vts.getRawParameterValue (r1Tag);
    r2Param = vts.getRawParameterValue (r2Tag);
}

void PulseShaper::addParameters (Parameters& params)
{
    params.push_back (std::make_unique<AudioParameterFloat> (r1Tag, "R1", 10000.0f, 200000.0f, 100000.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (r2Tag, "R2", 1000.0f, 10000.0f, 4700.0f));
}

void PulseShaper::reset (double sampleRate)
{
    using namespace chowdsp::WDF;

    c40 = std::make_unique<Capacitor> (0.015e-6, sampleRate, 0.029);
    P1 = std::make_unique<WDFParallel> (c40.get(), &r163);
    S1 = std::make_unique<WDFSeries> (&Vs, P1.get());

    I1 = std::make_unique<PolarityInverter> (&r162);
    P2 = std::make_unique<WDFParallel> (I1.get(), S1.get());

    d53.connectToNode (P2.get());
}

void PulseShaper::processBlock (float* block, const int numSamples)
{
    r162.setResistanceValue (*r2Param);
    r163.setResistanceValue (*r1Param);

    for (int n = 0; n < numSamples; ++n)
        block[n] = processSample (block[n]);
}
