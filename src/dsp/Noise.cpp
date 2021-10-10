#include "Noise.h"

using namespace NoiseTags;

Noise::Noise (AudioProcessorValueTreeState& vts)
{
    amtParam = vts.getRawParameterValue (amtTag);
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

    NormalisableRange<float> freqRange { 10.0f, 1000.0f };
    freqRange.setSkewForCentre (100.0f);
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

    buffer.setSize (1, samplesPerBlock);
}

void Noise::processBlock (dsp::AudioBlock<Vec>& block, int numSamples)
{
    noise.setGainLinear (*amtParam);
    noise.setNoiseType (NoiseType::Uniform);

    buffer.setSize (1, numSamples, false, false, true);
    buffer.clear();
    dsp::AudioBlock<float> noiseBlock { buffer };
    dsp::ProcessContextReplacing<float> context { noiseBlock };
    noise.process (context);

    filter.setCutoffFrequency (*freqParam);
    filter.process<decltype (context), chowdsp::StateVariableFilterType::Lowpass> (context);

    auto* blockData = block.getChannelPointer (0);
    auto* noiseData = noiseBlock.getChannelPointer (0);
    for (int n = 0; n < numSamples; ++n)
        blockData[n] += blockData[n] * noiseData[n];
}
