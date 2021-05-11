#include "PulseViewer.h"

namespace
{
constexpr double fs = 96000.0;
constexpr int nSamples = int (0.0032 * fs); // 3.2 milliseconds
constexpr int offset = int (0.0001 * fs); // 0.1 millisecond
}

PulseViewer::PulseViewer (AudioProcessorValueTreeState& vts) : trigger(vts),
                                                               shaper (vts, fs)
{
    trigger.prepareToPlay (fs, nSamples);
    startTimerHz (35);

    setColour (backgroundColour, Colours::black);
    setColour (traceColour, Colours::lightblue);
}

void PulseViewer::resized()
{
}

void PulseViewer::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColour));

    AudioBuffer<float> buffer (1, nSamples);
    buffer.clear();
    
    auto midiMessage = MidiMessage::noteOn (1, 64, (uint8) 127);
    MidiBuffer midiBuffer; 
    midiBuffer.addEvent (midiMessage, offset);

    trigger.processBlock (buffer, midiBuffer);
    shaper.processBlock (buffer.getWritePointer (0), nSamples);

    const auto yScale = (float) getHeight() * 0.73f;
    const auto yOff = (float) getHeight() * 0.27f;

    Path curvePath;
    bool started = false;
    for (int n = 0; n < nSamples; ++n)
    {
        auto xDraw = ((float) n / (float) nSamples) * (float) getWidth();
        auto yDraw = (float) getHeight() - (yScale * buffer.getSample (0, n) + yOff);

        if (! started)
        {
            curvePath.startNewSubPath (xDraw, yDraw);
            started = true;
        }
        else
        {
            curvePath.lineTo (xDraw, yDraw);
        }
    }

    g.setColour (findColour (traceColour));
    g.strokePath (curvePath, PathStrokeType (2.0f, PathStrokeType::JointStyle::curved));
}

void PulseViewer::timerCallback()
{
    repaint();
}
