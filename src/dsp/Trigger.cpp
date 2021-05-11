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

void Trigger::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    const auto pulseSamples = int (fs * (*widthParam / 1000.0f));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        FloatVectorOperations::fill (buffer.getWritePointer (ch), ampParam->load(), leftoverSamples);
    leftoverSamples = 0;

    for (const MidiMessageMetadata mm : midi)
    {
        auto message = mm.getMessage();
        if (message.isNoteOn())
        {
            const auto samplePosition = mm.samplePosition;
            int samplesToFill = jmin (pulseSamples, buffer.getNumSamples() - samplePosition);
            leftoverSamples = pulseSamples - samplesToFill;

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                FloatVectorOperations::fill (buffer.getWritePointer (ch) + samplePosition, ampParam->load(), samplesToFill);

            curFreqHz = (float) MidiMessage::getMidiNoteInHertz (message.getNoteNumber());
        }
    }

    midi.clear();
}
