#include "Trigger.h"

namespace
{
const String widthTag = "trig_width";
const String ampTag = "trig_amp";
const String voicesTag = "trig_voices";
} // namespace

Trigger::Trigger (AudioProcessorValueTreeState& vtState) : vts (vtState)
{
    widthParam = vts.getRawParameterValue (widthTag);
    ampParam = vts.getRawParameterValue (ampTag);

    vts.addParameterListener (voicesTag, this);
    parameterChanged (voicesTag, *vts.getRawParameterValue (voicesTag));
}

Trigger::~Trigger()
{
    vts.removeParameterListener (voicesTag, this);
}

void Trigger::parameterChanged (const String& paramID, float newValue)
{
    if (paramID != voicesTag)
        return;

    std::fill (leftoverSamples.begin(), leftoverSamples.end(), 0);
    numVoices = (size_t) newValue + 1;
    voiceIdx = 0;
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

    params.push_back (std::make_unique<AudioParameterChoice> (voicesTag,
                                                              "Voices",
                                                              StringArray { "1", "2", "3", "4" },
                                                              0));
}

void Trigger::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    fs = (float) sampleRate;
    std::fill (leftoverSamples.begin(), leftoverSamples.end(), 0);
}

void fillBlock (dsp::AudioBlock<Vec>& block, float value, int start, int numToFill, size_t channel)
{
    auto* x = block.getChannelPointer (0);
    for (int i = start; i < start + numToFill; ++i)
        x[i].set (channel, value);
}

void Trigger::processBlock (dsp::AudioBlock<Vec>& block, const int numSamples, MidiBuffer& midi)
{
    jassert (block.getNumChannels() == 1); // only single-channel buffers!

    const auto pulseSamples = int (fs * (*widthParam / 1000.0f));

    for (size_t i = 0; i < numVoices; ++i)
    {
        int samplesToFill = jmin (leftoverSamples[i], numSamples);
        fillBlock (block, ampParam->load(), 0, samplesToFill, i);
        leftoverSamples[i] -= samplesToFill;
    }

    for (const MidiMessageMetadata mm : midi)
    {
        auto message = mm.getMessage();
        if (message.isNoteOn())
        {
            const auto samplePosition = mm.samplePosition;
            auto samplesToFill = jmin (pulseSamples, numSamples - samplePosition);
            leftoverSamples[voiceIdx] = pulseSamples - samplesToFill;

            fillBlock (block, ampParam->load(), samplePosition, samplesToFill, voiceIdx);
            voiceIdx = (voiceIdx + 1) % numVoices;

            curFreqHz.set (voiceIdx, (float) tuning.frequencyForMidiNote (message.getNoteNumber()));
        }
    }

    midi.clear();
}

void Trigger::resetTuning()
{
    tuning = Tunings::Tuning();
    scaleData = {};
    scaleName = {};
    mappingData = {};
    mappingName = {};

    tuningListeners.call (&Listener::tuningChanged);
}

void Trigger::setScaleFile (const File& scaleFile)
{
    if (! scaleFile.existsAsFile())
        return;
    
    scaleData = scaleFile.loadFileAsString().toStdString();
    scaleName = scaleFile.getFileNameWithoutExtension();

    try
    {   
       tuning = Tunings::Tuning (Tunings::parseSCLData (scaleData), Tunings::parseKBMData (mappingData));
    }
    catch (Tunings::TuningError& e)
    {
       tuning = Tunings::Tuning();
    }
    
    tuningListeners.call (&Listener::tuningChanged);
}

void Trigger::setMappingFile (const File& mappingFile)
{
    if (! mappingFile.existsAsFile())
        return;
    
    mappingData = mappingFile.loadFileAsString().toStdString();
    mappingName = mappingFile.getFileNameWithoutExtension();

    try
    {   
       tuning = Tunings::Tuning (Tunings::parseSCLData (scaleData), Tunings::parseKBMData (mappingData));
    }
    catch (Tunings::TuningError& e)
    {
       tuning = Tunings::Tuning();
    }

    tuningListeners.call (&Listener::tuningChanged);
}

void Trigger::getTuningState (XmlElement* xml)
{
    xml->setAttribute ("scale_name", scaleName);
    xml->setAttribute ("scale_data", String (scaleData));
    xml->setAttribute ("mapping_name", mappingName);
    xml->setAttribute ("mapping_data", String (mappingData));
}

void Trigger::setTuningState (XmlElement* xml)
{
    scaleName = xml->getStringAttribute ("scale_name");
    scaleData = xml->getStringAttribute ("scale_data").toStdString();
    mappingName = xml->getStringAttribute ("mapping_name");
    mappingData = xml->getStringAttribute ("mapping_data").toStdString();

    try
    {   
       tuning = Tunings::Tuning (Tunings::parseSCLData (scaleData), Tunings::parseKBMData (mappingData));
    }
    catch (Tunings::TuningError& e)
    {
       tuning = Tunings::Tuning();
    }

    tuningListeners.call (&Listener::tuningChanged);
}
