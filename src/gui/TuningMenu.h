#pragma once

#include "ChowKick.h"

class TuningMenu : public ComboBox,
                   private Trigger::Listener,
                   private Timer
{
public:
    TuningMenu (Trigger& trig, AudioProcessorValueTreeState& vts);
    ~TuningMenu() override;

    void tuningChanged() override { refreshMenu(); }
    void tuningLoadError (const String& message) override;

    void refreshMenu();
    void resetMenuText();

private:
#if JUCE_IOS
    void refreshMenuIOS();
#endif

    void timerCallback() override;
    void addMTSOptionToMenu (PopupMenu& menu);

    Trigger& trigger;
    RangedAudioParameter& useMTSParam;
    bool wasMTSParamOn = false;
    bool wasMTSMasterConnected = false;

    std::shared_ptr<FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TuningMenu)
};

class TuningMenuItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (TuningMenuItem)

    TuningMenuItem (foleys::MagicGUIBuilder& builder, const ValueTree& node)
        : foleys::GuiItem (builder, node)
    {
        auto plugin = dynamic_cast<ChowKick*> (builder.getMagicState().getProcessor());
        comp = std::make_unique<TuningMenu> (plugin->getTrigger(), plugin->getVTS());
        addAndMakeVisible (comp.get());
    }

    void update() override
    {
        comp->resized();
    }

    Component* getWrappedComponent() override
    {
        return comp.get();
    }

private:
    std::unique_ptr<TuningMenu> comp;
};
