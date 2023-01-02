#include "Noise.h"

using namespace NoiseTags;

Noise::Noise (AudioProcessorValueTreeState& vts, bool allowParamMod) : allowParamModulation (allowParamMod)
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (amtParam, vts, amtTag);
    loadParameterPointer (decayParam, vts, decayTag);
    loadParameterPointer (freqParam, vts, freqTag);
    loadParameterPointer (typeParam, vts, typeTag);
}

void Noise::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createPercentParameter (params, amtTag, "Noise Amount", 0.0f);
    createPercentParameter (params, decayTag, "Noise Decay", 0.5f);
    createFreqParameter (params, freqTag, "Noise Cutoff", 20.0f, 20000.0f, 2000.0f, 500.0f);
    emplace_param<chowdsp::EnumChoiceParameter<NoiseMode>> (params,
                                                            typeTag,
                                                            "Noise Type",
                                                            NoiseType::Uniform);
}

void Noise::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec { sampleRate, (uint32) samplesPerBlock, 1 };
    noise.prepare (spec);
    noise.setRampDurationSeconds (0.05);

    filter.prepare (spec);

    noiseBuffer = chowdsp::AudioBlock<Vec> (noiseData, 1, (size_t) samplesPerBlock);

    decaySmooth.reset (sampleRate, 0.05);
}

void Noise::processBlock (chowdsp::AudioBlock<Vec>& block, int numSamples)
{
    noise.setGainLinear (std::pow (allowParamModulation ? amtParam->getCurrentValue() : amtParam->get(), 2.0f));
    noise.setNoiseType (typeParam->get());
    decaySmooth.setTargetValue (std::pow (1.0f - *decayParam, 2.5f) * 2.0f + 1.0f);

    auto noiseBlock = noiseBuffer.getSubBlock (0, (size_t) numSamples);
    noiseBlock.clear();

    chowdsp::ProcessContextReplacing<Vec> context { noiseBlock };
    noise.process (context);

    filter.setCutoffFrequency (allowParamModulation ? freqParam->getCurrentValue() : freqParam->get());
    filter.process<decltype (context)> (context);

    auto* blockData = block.getChannelPointer (0);
    auto* noisePtr = noiseBlock.getChannelPointer (0);

    for (int n = 0; n < numSamples; ++n)
    {
        auto noiseGain = xsimd::pow (xsimd::abs (blockData[n]), (Vec) decaySmooth.getNextValue());
        blockData[n] += noiseGain * noisePtr[n];
    }
}
