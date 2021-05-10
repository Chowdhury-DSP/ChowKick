#pragma once

#include <pch.h>

class Trigger
{
public:
    Trigger (AudioProcessorValueTreeState& vts);

    static void addParameters (Parameters& params);
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi);

    float getFrequencyHz() const noexcept { return curFreqHz; }

private:
    std::atomic<float>* widthParam = nullptr;
    std::atomic<float>* ampParam   = nullptr;

    float fs = 44100.0f;
    int leftoverSamples = 0;

    float curFreqHz = 10.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trigger)
};
