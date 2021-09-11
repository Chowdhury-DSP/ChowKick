#include "TuningMenu.h"

TuningMenu::TuningMenu (Trigger& trig) : trigger (trig)
{
    trigger.addListener (this);
    refreshMenu();

    setColour (ComboBox::backgroundColourId, Colours::transparentBlack);
    setJustificationType (Justification::centred);
}

TuningMenu::~TuningMenu()
{
    trigger.removeListener (this);
}

void TuningMenu::refreshMenu()
{
    constexpr auto fileChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

    auto* rootMenu = getRootMenu();
    rootMenu->clear();

    auto sclName = trigger.getScaleName();
    auto scaleOption = "Select SCL" + (sclName.isEmpty() ? "" : " (" + sclName + ")");
    rootMenu->addItem (scaleOption, [=]
                       {
                           resetMenuText();
                           FileChooser chooser ("Choose Scale", File(), "*.scl");
                           if (chooser.browseForFileToOpen())
                               trigger.setScaleFile (chooser.getResult());
                       });

    auto kbmName = trigger.getMappingName();
    auto mappingOption = "Select KBM" + (kbmName.isEmpty() ? "" : " (" + kbmName + ")");
    rootMenu->addItem (mappingOption, [=]
                       {
                           resetMenuText();
                           FileChooser chooser ("Choose Mapping", File(), "*.kbm");
                           if (chooser.browseForFileToOpen())
                               trigger.setMappingFile (chooser.getResult());
                       });

    rootMenu->addItem ("Reset to 12TET", [=]
                       { trigger.resetTuning(); });

    resetMenuText();
}

void TuningMenu::tuningLoadError (const String& message)
{
#if JUCE_IOS
    NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon, "Tuning Error", message);
#else
    AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Tuning Error", message);
#endif
}
