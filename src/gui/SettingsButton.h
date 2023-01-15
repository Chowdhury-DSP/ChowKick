#pragma once

#include <pch.h>

class ChowKick;
class SettingsButton : public DrawableButton,
                       private chowdsp::TrackedByBroadcasters
{
    using SettingID = chowdsp::GlobalPluginSettings::SettingID;

public:
    SettingsButton (ChowKick& processor, chowdsp::OpenGLHelper* openGLHelper);
    ~SettingsButton() override;

    void globalSettingChanged (SettingID settingID);

private:
    void showSettingsMenu();
    void openGLMenu (PopupMenu& menu, int itemID);
    void copyDiagnosticInfo();

    const ChowKick& proc;
    chowdsp::OpenGLHelper* openGLHelper;

    chowdsp::SharedPluginSettings pluginSettings;
    chowdsp::SharedLNFAllocator lnfAllocator;

    static constexpr SettingID openglID = "use_opengl";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsButton)
};

class SettingsButtonItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (SettingsButtonItem)

    SettingsButtonItem (foleys::MagicGUIBuilder& builder, const ValueTree& node);

    void update() override {}

    Component* getWrappedComponent() override
    {
        return button.get();
    }

private:
    std::unique_ptr<SettingsButton> button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsButtonItem)
};
