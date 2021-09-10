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

static File getTuningFile (const String& dialog, const String& filePatterns)
{
    FileChooser chooser (dialog, File(), filePatterns);
    if (chooser.browseForFileToOpen())
        return chooser.getResult();

    return {};
}

void TuningMenu::refreshMenu()
{
    auto* rootMenu = getRootMenu();
    rootMenu->clear();

    auto sclName = trigger.getScaleName();
    auto scaleOption = "Select SCL" + (sclName.isEmpty() ? "" : " (" + sclName + ")");
    rootMenu->addItem (scaleOption, [=] {
        auto sclFile = getTuningFile ("Choose Scale", "*.scl");
        trigger.setScaleFile (sclFile);
    });

    auto kbmName = trigger.getMappingName();
    auto mappingOption = "Select KBM" + (kbmName.isEmpty() ? "" : " (" + kbmName + ")");
    rootMenu->addItem (mappingOption, [=] {
        auto kbmFile = getTuningFile ("Choose Mapping", "*.kbm");
        trigger.setMappingFile (kbmFile);
    });

    rootMenu->addItem ("Reset to 12TET", [=] {
        trigger.resetTuning();
    });

    setText ("Tuning", dontSendNotification);
}
