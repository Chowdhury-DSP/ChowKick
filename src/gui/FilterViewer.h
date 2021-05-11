#pragma once

#include "ChowKick.h"
#include "FilterViewHelper.h"

class FilterViewer : public Component,
                     private Timer
{
public:
    FilterViewer (AudioProcessorValueTreeState& vts);

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
    ResonantFilter resFilter;

    FilterViewHelper helper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterViewer)
};

class FilterViewerItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (FilterViewerItem)

    FilterViewerItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
        : foleys::GuiItem (builder, node)
    {
        auto plugin = dynamic_cast<ChowKick*> (builder.getMagicState().getProcessor());
        comp = std::make_unique<FilterViewer> (plugin->getVTS());
        addAndMakeVisible (comp.get());

        setColourTranslation ({
            { "background", FilterViewer::backgroundColour },
            { "trace", FilterViewer::traceColour },
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
    std::unique_ptr<FilterViewer> comp;
};
