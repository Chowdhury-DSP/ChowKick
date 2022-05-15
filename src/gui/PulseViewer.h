#pragma once

#include "ChowKick.h"
#include "dsp/Noise.h"
#include "dsp/PulseShaper.h"
#include "dsp/Trigger.h"

class PulseViewer : public Component,
                    public SettableTooltipClient,
                    private AudioProcessorValueTreeState::Listener
{
public:
    PulseViewer (AudioProcessorValueTreeState& vts);
    ~PulseViewer() override;

    enum ColourIDs
    {
        backgroundColour,
        traceColour,
    };

    void resized() override;
    void paint (Graphics& g) override;

    void parameterChanged (const String&, float) override;
    void updatePath();

private:
    AudioProcessorValueTreeState& vts;

    Trigger trigger;
    Noise noise;
    std::unique_ptr<PulseShaper> shaper;

    Path pulsePath;

    HeapBlock<char> blockData;
    chowdsp::AudioBlock<Vec> block;

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
