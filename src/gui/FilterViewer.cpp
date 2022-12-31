#include "FilterViewer.h"

namespace
{
constexpr float lowFreq = 25.0f;
constexpr float highFreq = 750.0f;
constexpr float dbRange = 72.0f;
constexpr double fs = 48000.0;

float getFreqForX (float xNorm)
{
    return lowFreq * std::pow ((highFreq / lowFreq), xNorm);
}

float getXForFreq (float freq)
{
    return std::log (freq / lowFreq) / std::log (highFreq / lowFreq);
}

float getYForMagnitude (float mag, float height)
{
    auto magDB = jmin (Decibels::gainToDecibels (mag), 70.0f);
    auto normDB = magDB / dbRange;
    return height * (1.0f - 0.85f * normDB) - (height * 0.1f);
}
} // namespace

FilterViewer::FilterViewer (AudioProcessorValueTreeState& vtState) : vts (vtState),
                                                                     trigger (vts),
                                                                     resFilter (vts, trigger),
                                                                     helper (resFilter)
{
    setColour (backgroundColour, Colours::black);
    setColour (traceColour, Colours::lightblue);

    resFilter.reset (fs);

    vts.addParameterListener (ResTags::freqTag.getParamID(), this);
    vts.addParameterListener (ResTags::linkTag.getParamID(), this);
    vts.addParameterListener (ResTags::qTag.getParamID(), this);
    vts.addParameterListener (ResTags::dampTag.getParamID(), this);
    vts.addParameterListener (ResTags::tightTag.getParamID(), this);
    vts.addParameterListener (ResTags::bounceTag.getParamID(), this);
    vts.addParameterListener (ResTags::modeTag.getParamID(), this);
}

FilterViewer::~FilterViewer()
{
    vts.removeParameterListener (ResTags::freqTag.getParamID(), this);
    vts.removeParameterListener (ResTags::linkTag.getParamID(), this);
    vts.removeParameterListener (ResTags::qTag.getParamID(), this);
    vts.removeParameterListener (ResTags::dampTag.getParamID(), this);
    vts.removeParameterListener (ResTags::tightTag.getParamID(), this);
    vts.removeParameterListener (ResTags::bounceTag.getParamID(), this);
    vts.removeParameterListener (ResTags::modeTag.getParamID(), this);
}

void FilterViewer::resized()
{
    updatePath();
}

void FilterViewer::updatePath()
{
    const auto width = (float) getWidth();
    const auto height = (float) getHeight();
    const auto left = (float) getX();
    const auto top = (float) getY();

    helper.prepare();
    const auto resFreq = helper.getResFrequency();

    tracePath.clear();
    float traceMagnitude = helper.getMagForFreq (getFreqForX (0.0f));
    float yPos = getYForMagnitude (traceMagnitude, height);
    tracePath.startNewSubPath (left, top + yPos);

    tracePathNL.clear();
    float traceMagnitudeNL = helper.getMagForFreqNL (getFreqForX (0.0f));
    float yPosNL = getYForMagnitude (traceMagnitudeNL, height);
    tracePathNL.startNewSubPath (left, top + yPosNL);

    constexpr float xInc = 0.25f;
    bool beforeRes = false;
    for (float xPos = 1.0f; xPos < width; xPos += xInc)
    {
        traceMagnitude = helper.getMagForFreq (getFreqForX (xPos / width));
        yPos = getYForMagnitude (traceMagnitude, height);
        tracePath.lineTo (left + xPos, top + yPos);

        traceMagnitudeNL = helper.getMagForFreqNL (getFreqForX (xPos / width));
        yPosNL = getYForMagnitude (traceMagnitudeNL, height);
        tracePathNL.lineTo (left + xPos, top + yPosNL);

        if (beforeRes && getFreqForX ((xPos + xInc) / width) >= resFreq)
        {
            auto xP = width * getXForFreq (resFreq);

            traceMagnitude = helper.getMagForFreq (resFreq);
            yPos = getYForMagnitude (traceMagnitude, height);
            tracePath.lineTo (left + xP, top + yPos);

            traceMagnitudeNL = helper.getMagForFreqNL (resFreq);
            yPosNL = getYForMagnitude (traceMagnitudeNL, height);
            tracePathNL.lineTo (left + xP, top + yPosNL);

            beforeRes = true;
        }
    }

    repaint();
}

void FilterViewer::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColour));

    g.setColour (Colours::red);
    g.strokePath (tracePathNL, PathStrokeType (2.0f, PathStrokeType::JointStyle::curved));

    g.setColour (findColour (traceColour));
    g.strokePath (tracePath, PathStrokeType (2.0f, PathStrokeType::JointStyle::curved));
}

void FilterViewer::parameterChanged (const String&, float)
{
    MessageManager::callAsync ([=]
                               { updatePath(); });
}
