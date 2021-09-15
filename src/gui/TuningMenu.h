#pragma once

#include "ChowKick.h"

class TuningMenu : public ComboBox,
                   private Trigger::Listener
{
public:
    TuningMenu (Trigger& trig);
    ~TuningMenu() override;

    void tuningChanged() override { refreshMenu(); }
    void tuningLoadError (const String& message) override;

    void refreshMenu();
    void resetMenuText() { setText ("Tuning", dontSendNotification); }

private:
    Trigger& trigger;

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
        comp = std::make_unique<TuningMenu> (plugin->getTrigger());
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
