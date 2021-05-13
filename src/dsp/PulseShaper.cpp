#include "PulseShaper.h"

namespace
{
    const String sustainTag = "pulse_sustain";
    const String decayTag = "pulse_decay";
}

PulseShaper::PulseShaper (AudioProcessorValueTreeState& vts, double sampleRate) :
    c40 (0.015e-6f, (float) sampleRate, 0.029f)
{
    d53.connectToNode (&P2);

    decayParam = vts.getRawParameterValue (decayTag);
    sustainParam = vts.getRawParameterValue (sustainTag);
}

void PulseShaper::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;

    params.push_back (std::make_unique<VTSParam> (sustainTag,
                                                  "Sustain",
                                                  String(),
                                                  NormalisableRange<float> { 0.0f, 1.0f },
                                                  0.5f,
                                                  &percentValToString,
                                                  &stringToPercentVal));
    params.push_back (std::make_unique<VTSParam> (decayTag,
                                                  "Decay",
                                                  String(),
                                                  NormalisableRange<float> { 0.0f, 1.0f },
                                                  0.5f,
                                                  &percentValToString,
                                                  &stringToPercentVal));
}

void PulseShaper::processBlock (dsp::AudioBlock<Vec>& block, const int numSamples)
{
    constexpr float r1Off = 5000.0f;
    constexpr float r1Scale = 500000.0f;
    auto sustainVal = 1.0f - std::pow (sustainParam->load(), 0.05f);
    auto r1Val = r1Off + (sustainVal * (r1Scale - r1Off));
    r163.setResistanceValue (r1Val);

    constexpr float r2Off = 500.0f;
    constexpr float r2Scale = 100000.0f;
    auto decayVal = std::pow (decayParam->load(), 2.0f);
    auto r2Val = r2Off + (decayVal * (r2Scale - r2Off));
    r162.setResistanceValue (r2Val);

    auto* x = block.getChannelPointer (0);
    for (int n = 0; n < numSamples; ++n)
        x[n] = processSample (x[n]);
}
