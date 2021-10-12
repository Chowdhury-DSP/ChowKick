#include "PulseViewer.h"

namespace
{
constexpr double fs = 96000.0;
constexpr int nSamples = int (0.0032 * fs); // 3.2 milliseconds
constexpr int offset = int (0.0001 * fs); // 0.1 millisecond
} // namespace

PulseViewer::PulseViewer (AudioProcessorValueTreeState& vtState) : vts (vtState),
                                                                   trigger (vts),
                                                                   noise (vts),
                                                                   block (blockData, 1, nSamples)
{
    vts.addParameterListener (TriggerTags::ampTag, this);
    vts.addParameterListener (TriggerTags::widthTag, this);
    vts.addParameterListener (ShaperTags::decayTag, this);
    vts.addParameterListener (ShaperTags::sustainTag, this);
    vts.addParameterListener (NoiseTags::amtTag, this);
    vts.addParameterListener (NoiseTags::freqTag, this);
    vts.addParameterListener (NoiseTags::typeTag, this);

    setColour (backgroundColour, Colours::black);
    setColour (traceColour, Colours::lightblue);
}

PulseViewer::~PulseViewer()
{
    vts.removeParameterListener (TriggerTags::ampTag, this);
    vts.removeParameterListener (TriggerTags::widthTag, this);
    vts.removeParameterListener (ShaperTags::decayTag, this);
    vts.removeParameterListener (ShaperTags::sustainTag, this);
    vts.removeParameterListener (NoiseTags::amtTag, this);
    vts.removeParameterListener (NoiseTags::freqTag, this);
    vts.removeParameterListener (NoiseTags::typeTag, this);
}

void PulseViewer::resized()
{
    updatePath();
}

void PulseViewer::updatePath()
{
    trigger.prepareToPlay (fs, nSamples);
    noise.prepareToPlay (fs, nSamples);
    shaper = std::make_unique<PulseShaper> (vts, fs);

    block.clear();
    auto midiMessage = MidiMessage::noteOn (1, 64, (uint8) 127);
    MidiBuffer midiBuffer;
    midiBuffer.addEvent (midiMessage, offset);

    trigger.processBlock (block, nSamples, midiBuffer);
    shaper->processBlock (block, nSamples);
    noise.processBlock (block, nSamples);

    const auto yScale = proportionOfHeight (0.6f);
    const auto yOff = proportionOfHeight (0.3f);

    pulsePath.clear();
    bool started = false;
    for (int n = 0; n < nSamples; ++n)
    {
        auto xDraw = ((float) n / (float) nSamples) * (float) getWidth();
        auto yDraw = (float) getHeight() - (yScale * jmin (block.getSample (0, n).sum(), 1.2f) + yOff) + 4.0f;

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
    MessageManager::callAsync ([=]
                               { updatePath(); });
}
