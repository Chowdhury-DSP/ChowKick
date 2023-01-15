#include "PulseViewer.h"

namespace
{
constexpr double fs = 96000.0;
constexpr int nSamples = int (0.0032 * fs); // 3.2 milliseconds
constexpr int offset = int (0.0001 * fs); // 0.1 millisecond
} // namespace

PulseViewer::PulseViewer (AudioProcessorValueTreeState& vtState) : vts (vtState),
                                                                   trigger (vts, false, false),
                                                                   noise (vts, false),
                                                                   shaper (vts, fs, false),
                                                                   block (blockData, 1, nSamples)
{
    vts.addParameterListener (TriggerTags::ampTag.getParamID(), this);
    vts.addParameterListener (TriggerTags::widthTag.getParamID(), this);
    vts.addParameterListener (ShaperTags::decayTag.getParamID(), this);
    vts.addParameterListener (ShaperTags::sustainTag.getParamID(), this);
    vts.addParameterListener (NoiseTags::amtTag.getParamID(), this);
    vts.addParameterListener (NoiseTags::freqTag.getParamID(), this);
    vts.addParameterListener (NoiseTags::typeTag.getParamID(), this);

    setColour (backgroundColour, Colours::black);
    setColour (traceColour, Colours::lightblue);
}

PulseViewer::~PulseViewer()
{
    vts.removeParameterListener (TriggerTags::ampTag.getParamID(), this);
    vts.removeParameterListener (TriggerTags::widthTag.getParamID(), this);
    vts.removeParameterListener (ShaperTags::decayTag.getParamID(), this);
    vts.removeParameterListener (ShaperTags::sustainTag.getParamID(), this);
    vts.removeParameterListener (NoiseTags::amtTag.getParamID(), this);
    vts.removeParameterListener (NoiseTags::freqTag.getParamID(), this);
    vts.removeParameterListener (NoiseTags::typeTag.getParamID(), this);
}

void PulseViewer::resized()
{
    updatePath();
}

void PulseViewer::updatePath()
{
    trigger.prepareToPlay (fs, nSamples);
    noise.prepareToPlay (fs, nSamples);
    shaper.reset();

    block.clear();
    auto midiMessage = MidiMessage::noteOn (1, 64, (uint8) 64);
    MidiBuffer midiBuffer;
    midiBuffer.addEvent (midiMessage, offset);

    trigger.processBlock (block, nSamples, midiBuffer);
    shaper.processBlock (block, nSamples);
    noise.processBlock (block, nSamples);

    const auto yScale = proportionOfHeight (0.6f);
    const auto yOff = proportionOfHeight (0.3f);

    pulsePath.clear();
    bool started = false;
    for (int n = 0; n < nSamples; ++n)
    {
        auto xDraw = ((float) n / (float) nSamples) * (float) getWidth();
        auto yDraw = (float) getHeight() - (yScale * jmin (xsimd::reduce_add (block.getSample (0, n)), 1.2f) + yOff) + 4.0f;

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
