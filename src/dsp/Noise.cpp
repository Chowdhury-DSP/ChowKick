#include "Noise.h"

using namespace NoiseTags;

Noise::Noise (AudioProcessorValueTreeState& vts)
{
    amtParam = vts.getRawParameterValue (amtTag);
    decayParam = vts.getRawParameterValue (decayTag);
    freqParam = vts.getRawParameterValue (freqTag);
    typeParam = vts.getRawParameterValue (typeTag);
}

void Noise::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;

    params.push_back (std::make_unique<VTSParam> (amtTag,
                                                  "Noise Amount",
                                                  String(),
                                                  NormalisableRange<float> { 0.0f, 1.0f },
                                                  0.0f,
                                                  &percentValToString,
                                                  &stringToPercentVal));

    params.push_back (std::make_unique<VTSParam> (decayTag,
                                                  "Noise Decay",
                                                  String(),
                                                  NormalisableRange<float> { 0.0f, 1.0f },
                                                  0.5f,
                                                  &percentValToString,
                                                  &stringToPercentVal));

    NormalisableRange<float> freqRange { 20.0f, 20000.0f };
    freqRange.setSkewForCentre (2000.0f);
    params.push_back (std::make_unique<VTSParam> (freqTag,
                                                  "Noise Cutoff",
                                                  String(),
                                                  freqRange,
                                                  500.0f,
                                                  &freqValToString,
                                                  &stringToFreqVal));

    params.push_back (std::make_unique<AudioParameterChoice> (typeTag,
                                                              "Noise Type",
                                                              StringArray { "Uniform", "Normal", "Pink" },
                                                              0));
}

void Noise::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec { sampleRate, (uint32) samplesPerBlock, 1 };
    noise.prepare (spec);
    noise.setRampDurationSeconds (0.05);

    filter.prepare (spec);

    noiseBuffer = dsp::AudioBlock<Vec> (noiseData, 1, samplesPerBlock);

    decaySmooth.reset (sampleRate, 0.05);
}

void Noise::processBlock (dsp::AudioBlock<Vec>& block, int numSamples)
{
    noise.setGainLinear (std::pow (*amtParam, 2.0f));
    noise.setNoiseType (NoiseType::Uniform);
    decaySmooth.setTargetValue (std::pow (1.0f - *decayParam, 2.5f) * 2.0f + 1.0f);

    auto noiseBlock = noiseBuffer.getSubBlock (0, numSamples);
    noiseBlock.clear();

    dsp::ProcessContextReplacing<Vec> context { noiseBlock };
    noise.process (context);

    filter.setCutoffFrequency (*freqParam);
    filter.process<decltype (context), chowdsp::StateVariableFilterType::Lowpass> (context);

    auto* blockData = block.getChannelPointer (0);
    auto* noisePtr = noiseBlock.getChannelPointer (0);
    
    for (int n = 0; n < numSamples; ++n)
    {
        auto noiseGain = chowdsp::SIMDUtils::powSIMD (Vec::abs (blockData[n]), (Vec) decaySmooth.getNextValue());
        blockData[n] += noiseGain * noisePtr[n];
    }
}
