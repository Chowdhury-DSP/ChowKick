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
    NormalisableRange<float> pulseRange (0.01f, 10.0f);
    pulseRange.setSkewForCentre (1.0f);

    params.push_back (std::make_unique<AudioParameterFloat> (widthTag, "Pulse Width [ms]", pulseRange, 1.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (ampTag, "Pulse Amp", 0.0f, 1.0f, 5.0f));
}

void Trigger::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    fs = (float) sampleRate;
    leftoverSamples = 0;
}

void Trigger::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    pulseSamples = int (fs * (*widthParam / 1000.0f));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        FloatVectorOperations::fill (buffer.getWritePointer (ch), ampParam->load(), leftoverSamples);
    leftoverSamples = 0;

    int samplePosition = 0;
    for (const MidiMessageMetadata mm : midi)
    {
        auto message = mm.getMessage();
        if (message.isNoteOn())
        {
            int samplesToFill = jmin (pulseSamples, buffer.getNumSamples() - samplePosition);
            leftoverSamples = pulseSamples - samplesToFill;

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                FloatVectorOperations::fill (buffer.getWritePointer (ch), ampParam->load(), samplesToFill);

            curFreqHz = (float) MidiMessage::getMidiNoteInHertz (message.getNoteNumber());
        }
    }

    midi.clear();
}
