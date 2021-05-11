#pragma once

#include "ChowKick.h"
#include "dsp/Trigger.h"
#include "dsp/PulseShaper.h"

class PulseViewer : public Component,
                    private Timer
{
public:
    PulseViewer (AudioProcessorValueTreeState& vts);

    enum ColourIDs
    {
        backgroundColour,
        traceColour,
    };

    void resized() override;
    void paint (Graphics& g) override;

    void timerCallback() override;

private:
    Trigger trigger;
    PulseShaper shaper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulseViewer)
};

class PulseViewerItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (PulseViewerItem)

    PulseViewerItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
        : foleys::GuiItem (builder, node)
    {
        auto plugin = dynamic_cast<ChowKick*> (builder.getMagicState().getProcessor());
        comp = std::make_unique<PulseViewer> (plugin->getVTS());
        addAndMakeVisible (comp.get());

        setColourTranslation ({
            { "background", PulseViewer::backgroundColour },
            { "trace", PulseViewer::traceColour },
        });
    }

    void update() override
    {
        comp->resized();
    }

    juce::Component* getWrappedComponent() override
    {
        return comp.get();
    }

private:
    std::unique_ptr<PulseViewer> comp;
};
