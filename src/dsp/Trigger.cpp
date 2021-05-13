#include "Trigger.h"

namespace
{
    const String widthTag = "trig_width";
    const String ampTag   = "trig_amp";


}

Trigger::Trigger (AudioProcessorValueTreeState& vts)
{
    widthParam = vts.getRawParameterValue (widthTag);
    ampParam   = vts.getRawParameterValue (ampTag);
}

void Trigger::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;

    NormalisableRange<float> pulseRange (0.025f, 2.5f);
    pulseRange.setSkewForCentre (1.0f);

    params.push_back (std::make_unique<VTSParam> (widthTag,
                                                  "Pulse Width [ms]",
                                                  String(),
                                                  pulseRange,
                                                  1.0f,
                                                  &timeMsValToString,
                                                  &stringToTimeMsVal));
    params.push_back (std::make_unique<VTSParam> (ampTag,
                                                  "Pulse Amp",
                                                  String(),
                                                  NormalisableRange<float> { 0.0f, 1.0f },
                                                  1.0f,
                                                  &percentValToString,
                                                  &stringToPercentVal));
}

void Trigger::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    fs = (float) sampleRate;
    leftoverSamples = 0;
}

void fillBlock (dsp::AudioBlock<Vec>& block, float value, int start, int numToFill)
{
    auto* x = block.getChannelPointer (0);
    for (int i = start; i < start + numToFill; ++i)
        x[i] = value;
}

void Trigger::processBlock (dsp::AudioBlock<Vec>& block, const int numSamples, MidiBuffer& midi)
{
    jassert (block.getNumChannels() == 1); // only single-channel buffers!

    const auto pulseSamples = int (fs * (*widthParam / 1000.0f));

    int samplesToFill = jmin (leftoverSamples, numSamples);
    fillBlock (block, ampParam->load(), 0, samplesToFill);
    leftoverSamples -= samplesToFill;

    for (const MidiMessageMetadata mm : midi)
    {
        auto message = mm.getMessage();
        if (message.isNoteOn())
        {
            const auto samplePosition = mm.samplePosition;
            samplesToFill = jmin (pulseSamples, numSamples - samplePosition);
            leftoverSamples = pulseSamples - samplesToFill;

            fillBlock (block, ampParam->load(), samplePosition, samplesToFill);
            
            curFreqHz = (float) MidiMessage::getMidiNoteInHertz (message.getNoteNumber());
        }
    }

    midi.clear();
}
