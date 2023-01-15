#pragma once

#include <pch.h>

namespace TriggerTags
{
const juce::ParameterID widthTag { "trig_width", VersionHints::original };
const juce::ParameterID ampTag { "trig_amp", VersionHints::original };
const juce::ParameterID voicesTag { "trig_voices", VersionHints::original };
const juce::ParameterID useMTSTag { "use_mts", VersionHints::v1_2_0 };
const juce::ParameterID enableVelocitySenseTag { "velo_sense", VersionHints::v1_2_0 };
} // namespace TriggerTags

class Trigger
{
public:
    explicit Trigger (AudioProcessorValueTreeState& vts,
                      bool allowParamModulation = true,
                      bool initialiseMTSClient = true);
    ~Trigger();

    static void addParameters (Parameters& params);
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (chowdsp::AudioBlock<Vec>& block, int numSamples, MidiBuffer& midi);

    Vec getFrequencyHz() const noexcept { return curFreqHz; }

    void resetTuning();
    void setScaleFile (const File& scaleFile);
    void setMappingFile (const File& mappingFile);
    void setTuningFromScaleAndMappingData();

    String getScaleName() const noexcept { return scaleName; }
    String getMappingName() const noexcept { return mappingName; }
    bool isMTSAvailable() const noexcept;
    bool isCurrentlyUsingMTS() const noexcept;

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
    void setNumVoices();
    float getAmp() const noexcept;
    float getWidth() const noexcept;

    chowdsp::FloatParameter* widthParam = nullptr;
    chowdsp::FloatParameter* ampParam = nullptr;
    chowdsp::ChoiceParameter* voicesParam = nullptr;
    chowdsp::BoolParameter* useMTSParam = nullptr;
    chowdsp::BoolParameter* useVeloSenseParam = nullptr;
    const bool allowParamModulation = true;

    float fs = 44100.0f;
    Vec curFreqHz = 10.0f;

    size_t voiceIdx = 0;
    size_t numVoices = 0;
    std::array<int, Vec::size> leftoverSamples {};
    std::array<float, Vec::size> velocityAmplitudes { 1.0f, 1.0f, 1.0f, 1.0f };

    Tunings::Tuning tuning;
    ListenerList<Listener> tuningListeners;
    String scaleName;
    std::string scaleData;
    String mappingName;
    std::string mappingData;

#if CHOW_KICK_WITH_MTS
    MTSClient* mtsClient = nullptr;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trigger)
};
