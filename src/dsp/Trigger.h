#pragma once

#include <pch.h>

class Trigger : private AudioProcessorValueTreeState::Listener
{
public:
    Trigger (AudioProcessorValueTreeState& vts);
    ~Trigger() override;

    static void addParameters (Parameters& params);
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (dsp::AudioBlock<Vec>& block, int numSamples, MidiBuffer& midi);

    void parameterChanged (const String& paramID, float newValue) override;

    Vec getFrequencyHz() const noexcept { return curFreqHz; }

    void resetTuning();
    void setScaleFile (const File& scaleFile);
    void setMappingFile (const File& mappingFile);
    void setTuningFromScaleAndMappingData ();

    String getScaleName() const noexcept { return scaleName; }
    String getMappingName() const noexcept { return mappingName; }

    void getTuningState (XmlElement* xml);
    void setTuningState (XmlElement* xml);

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void tuningChanged() {}
        virtual void tuningLoadError (const String& /*message*/) {}
    };

    void addListener (Listener* l) { tuningListeners.add (l); }
    void removeListener (Listener* l) { tuningListeners.remove (l); }

private:
    std::atomic<float>* widthParam = nullptr;
    std::atomic<float>* ampParam = nullptr;
    AudioProcessorValueTreeState& vts;

    float fs = 44100.0f;
    Vec curFreqHz = 10.0f;

    size_t voiceIdx = 0;
    size_t numVoices;
    std::array<int, Vec::size()> leftoverSamples { 0 };

    Tunings::Tuning tuning;
    ListenerList<Listener> tuningListeners;
    String scaleName;
    std::string scaleData;
    String mappingName;
    std::string mappingData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trigger)
};
