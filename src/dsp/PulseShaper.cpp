#include "PulseShaper.h"

using namespace ShaperTags;

PulseShaper::PulseShaper (AudioProcessorValueTreeState& vts, double sampleRate) : c40 (0.015e-6f, (float) sampleRate, 0.029f)
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (decayParam, vts, decayTag);
    loadParameterPointer (sustainParam, vts, sustainTag);
}

void PulseShaper::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createPercentParameter (params, sustainTag, "Sustain", 0.5f);
    createPercentParameter (params, decayTag, "Decay", 0.5f);
}

void PulseShaper::processBlock (chowdsp::AudioBlock<Vec>& block, const int numSamples)
{
    constexpr float r1Off = 5000.0f;
    constexpr float r1Scale = 500000.0f;
    auto sustainVal = 1.0f - std::pow (sustainParam->getCurrentValue(), 0.05f);
    auto r1Val = r1Off + (sustainVal * (r1Scale - r1Off));
    r163.setResistanceValue (r1Val);

    constexpr float r2Off = 500.0f;
    constexpr float r2Scale = 100000.0f;
    auto decayVal = std::pow (decayParam->getCurrentValue(), 2.0f);
    auto r2Val = r2Off + (decayVal * (r2Scale - r2Off));
    r162.setResistanceValue (r2Val);

    auto* x = block.getChannelPointer (0);
    for (int n = 0; n < numSamples; ++n)
        x[n] = processSample (x[n]);
}
