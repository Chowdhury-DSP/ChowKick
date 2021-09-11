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
    rootMenu->addItem (scaleOption, [=] {
        fileChooser = std::make_unique<FileChooser> ("Choose Scale", File(), "*.scl");
        fileChooser->launchAsync (fileChooserFlags, [=] (const FileChooser& fc) {
            trigger.setScaleFile (fc.getResult());
        });
    });

    auto kbmName = trigger.getMappingName();
    auto mappingOption = "Select KBM" + (kbmName.isEmpty() ? "" : " (" + kbmName + ")");
    rootMenu->addItem (mappingOption, [=] {
        fileChooser = std::make_unique<FileChooser> ("Choose Mapping", File(), "*.kbm");
        fileChooser->launchAsync (fileChooserFlags, [=] (const FileChooser& fc) {
            trigger.setMappingFile (fc.getResult());
        });
    });

    rootMenu->addItem ("Reset to 12TET", [=] {
        trigger.resetTuning();
    });

    setText ("Tuning", dontSendNotification);
}
