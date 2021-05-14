#include "PulseViewer.h"

namespace
{
constexpr double fs = 96000.0;
constexpr int nSamples = int (0.0032 * fs); // 3.2 milliseconds
constexpr int offset = int (0.0001 * fs); // 0.1 millisecond
} // namespace

PulseViewer::PulseViewer (AudioProcessorValueTreeState& vts) : trigger (vts),
                                                               shaper (vts, fs),
                                                               block (blockData, 1, nSamples)
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

    block.clear();
    auto midiMessage = MidiMessage::noteOn (1, 64, (uint8) 127);
    MidiBuffer midiBuffer;
    midiBuffer.addEvent (midiMessage, offset);

    trigger.processBlock (block, nSamples, midiBuffer);
    shaper.processBlock (block, nSamples);

    const auto yScale = (float) getHeight() * 0.68f;
    const auto yOff = (float) getHeight() * 0.3f;

    Path curvePath;
    bool started = false;
    for (int n = 0; n < nSamples; ++n)
    {
        auto xDraw = ((float) n / (float) nSamples) * (float) getWidth();
        auto yDraw = (float) getHeight() - (yScale * block.getSample (0, n).sum() + yOff);

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
