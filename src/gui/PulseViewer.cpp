#include "PulseViewer.h"

namespace
{
constexpr double fs = 96000.0;
constexpr int nSamples = int (0.0032 * fs); // 3.2 milliseconds
constexpr int offset = int (0.0001 * fs); // 0.1 millisecond
} // namespace

PulseViewer::PulseViewer (AudioProcessorValueTreeState& vtState) : vts (vtState),
                                                                   trigger (vts),
                                                                   shaper (vts, fs),
                                                                   block (blockData, 1, nSamples)
{
    trigger.prepareToPlay (fs, nSamples);

    vts.addParameterListener (TriggerTags::ampTag, this);
    vts.addParameterListener (TriggerTags::widthTag, this);
    vts.addParameterListener (ShaperTags::decayTag, this);
    vts.addParameterListener (ShaperTags::sustainTag, this);

    setColour (backgroundColour, Colours::black);
    setColour (traceColour, Colours::lightblue);
}

PulseViewer::~PulseViewer()
{
    vts.removeParameterListener (TriggerTags::ampTag, this);
    vts.removeParameterListener (TriggerTags::widthTag, this);
    vts.removeParameterListener (ShaperTags::decayTag, this);
    vts.removeParameterListener (ShaperTags::sustainTag, this);
}

void PulseViewer::resized()
{
    updatePath();
}

void PulseViewer::updatePath()
{
    block.clear();
    auto midiMessage = MidiMessage::noteOn (1, 64, (uint8) 127);
    MidiBuffer midiBuffer;
    midiBuffer.addEvent (midiMessage, offset);

    trigger.processBlock (block, nSamples, midiBuffer);
    shaper.processBlock (block, nSamples);

    const auto yScale = (float) getHeight() * 0.68f;
    const auto yOff = (float) getHeight() * 0.3f;

    pulsePath.clear();
    bool started = false;
    for (int n = 0; n < nSamples; ++n)
    {
        auto xDraw = ((float) n / (float) nSamples) * (float) getWidth();
        auto yDraw = (float) getHeight() - (yScale * block.getSample (0, n).sum() + yOff) + 4.0f;

        if (! started)
        {
            pulsePath.startNewSubPath (xDraw, yDraw);
            started = true;
        }
        else
        {
            pulsePath.lineTo (xDraw, yDraw);
        }
    }

    repaint();
}

void PulseViewer::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColour));

    g.setColour (findColour (traceColour));
    g.strokePath (pulsePath, PathStrokeType (2.0f, PathStrokeType::JointStyle::curved));
}

void PulseViewer::parameterChanged (const String&, float)
{
    updatePath();
}
