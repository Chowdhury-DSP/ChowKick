#pragma once

#include "../ChowKick.h"
#include "PresetManager.h"

class PresetComp : public Component,
                   private chowdsp::PresetManager::Listener
{
public:
    enum ColourIDs
    {
        backgroundColourId,
        textColourId,
    };

    PresetComp (ChowKick& proc, chowdsp::PresetManager& manager);
    ~PresetComp() override;

    void paint (Graphics& g) override;
    void resized() override;

    void presetListUpdated() override;
    void presetDirtyStatusChanged() override { updatePresetBoxText(); }
    void selectedPresetChanged() override { updatePresetBoxText(); }

private:
    void chooseUserPresetFolder();
    void addPresetOptions (int optionID);
    void saveUserPreset();
    void updatePresetBoxText();

    ChowKick& proc;
    chowdsp::PresetManager& manager;

    ComboBox presetBox;
    TextEditor presetNameEditor;

    DrawableButton presetsLeft, presetsRight;

    WaitableEvent waiter;
    std::shared_ptr<FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetComp)
};

class PresetComponentItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (PresetComponentItem)

    PresetComponentItem (foleys::MagicGUIBuilder& builder, const ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({ { "presets-background", PresetComp::backgroundColourId },
                                { "presets-text", PresetComp::textColourId } });

        if (auto* proc = dynamic_cast<ChowKick*> (builder.getMagicState().getProcessor()))
        {
            presetComp = std::make_unique<PresetComp> (*proc, proc->getPresetManager());
            addAndMakeVisible (presetComp.get());
        }
    }

    void update() override {}

    Component* getWrappedComponent() override
    {
        return presetComp.get();
    }

private:
    std::unique_ptr<PresetComp> presetComp;
};
