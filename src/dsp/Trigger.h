#pragma once

#include <pch.h>

class Trigger : private AudioProcessorValueTreeState::Listener
{
public:
    Trigger (AudioProcessorValueTreeState& vts);
    ~Trigger() override;

    static void addParameters (Parameters& params);
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (dsp::AudioBlock<Vec>& block, const int numSamples, MidiBuffer& midi);

    void parameterChanged (const String& paramID, float newValue) override;

    Vec getFrequencyHz() const noexcept { return curFreqHz; }

private:
    std::atomic<float>* widthParam = nullptr;
    std::atomic<float>* ampParam = nullptr;
    AudioProcessorValueTreeState& vts;

    float fs = 44100.0f;
    Vec curFreqHz = 10.0f;

    size_t voiceIdx = 0;
    size_t numVoices;
    std::array<int, Vec::size()> leftoverSamples { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trigger)
};
