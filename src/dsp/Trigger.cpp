#include "Trigger.h"

using namespace TriggerTags;

namespace
{
inline Vec insert (const Vec& v, float s, size_t i) noexcept
{
    union UnionType
    {
        Vec v;
        float s[4];
    };
    UnionType u { v };

    u.s[i] = s;
    return u.v;
}
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

void fillBlock (chowdsp::AudioBlock<Vec>& block, float value, int start, int numToFill, size_t channel)
{
    auto* x = block.getChannelPointer (0);
    for (int i = start; i < start + numToFill; ++i)
        x[i] = insert (x[i], value, channel);
}

void Trigger::processBlock (chowdsp::AudioBlock<Vec>& block, const int numSamples, MidiBuffer& midi)
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

            voiceIdx = (voiceIdx + 1) % numVoices;
            fillBlock (block, ampParam->load(), samplePosition, samplesToFill, voiceIdx);
            curFreqHz = insert (curFreqHz, (float) tuning.frequencyForMidiNote (message.getNoteNumber()), voiceIdx);

            leftoverSamples[voiceIdx] = pulseSamples - samplesToFill;
        }
    }

    midi.clear();
}

void Trigger::resetTuning()
{
    scaleData = {};
    scaleName = {};
    mappingData = {};
    mappingName = {};

    setTuningFromScaleAndMappingData();
    tuningListeners.call (&Listener::tuningChanged);
}

void Trigger::setTuningFromScaleAndMappingData()
{
    auto scale = Tunings::evenTemperament12NoteScale();
    auto mapping = Tunings::startScaleOnAndTuneNoteTo (60, 60, Tunings::MIDI_0_FREQ * 32);

    // Each of the scale and mapping can be set independently so parse them independently
    try
    {
        if (scaleName.isNotEmpty())
        {
            auto parseScale = Tunings::parseSCLData (scaleData);
            scale = parseScale;
            scaleName = scale.description;
        }
    }
    catch (Tunings::TuningError& e)
    {
        scaleName = {};
        scaleData = {};
        tuningListeners.call (&Listener::tuningLoadError, e.what());
    }

    try
    {
        if (mappingName.isNotEmpty())
        {
            auto parseMapping = Tunings::parseKBMData (mappingData);
            mapping = parseMapping;
        }
    }
    catch (Tunings::TuningError& e)
    {
        mappingName = {};
        mappingData = {};
        tuningListeners.call (&Listener::tuningLoadError, e.what());
    }

    // Then retune to those objects.
    try
    {
        tuning = Tunings::Tuning (scale, mapping);
    }
    catch (Tunings::TuningError& e)
    {
        tuning = Tunings::Tuning();
        tuningListeners.call (&Listener::tuningLoadError, e.what());
    }

    tuningListeners.call (&Listener::tuningChanged);
}

void Trigger::setScaleFile (const File& scaleFile)
{
    if (! scaleFile.existsAsFile())
        return;

    scaleData = scaleFile.loadFileAsString().toStdString();
    scaleName = scaleFile.getFileNameWithoutExtension();

    setTuningFromScaleAndMappingData();
}

void Trigger::setMappingFile (const File& mappingFile)
{
    if (! mappingFile.existsAsFile())
        return;

    mappingData = mappingFile.loadFileAsString().toStdString();
    mappingName = mappingFile.getFileNameWithoutExtension();

    setTuningFromScaleAndMappingData();
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

    setTuningFromScaleAndMappingData();
}
