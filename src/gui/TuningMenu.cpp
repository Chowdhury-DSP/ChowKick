#include "TuningMenu.h"

namespace
{
File getFactoryTuningLibrary()
{
    auto getLibraryPath = [] (const File& rootPath, const File& usrPath, const String& libPath)
    {
        auto libraryDir = rootPath.getChildFile (libPath);
        if (libraryDir.isDirectory())
            return libraryDir;

        libraryDir = usrPath.getChildFile (libPath);
        if (libraryDir.isDirectory())
            return libraryDir;

        return File();
    };

#if JUCE_WINDOWS
    const String libraryPath = "ChowKick/tuning_library";
    const auto rootPath = File::getSpecialLocation (File::commonApplicationDataDirectory);
    const auto usrPath = File::getSpecialLocation (File::userApplicationDataDirectory);
    return getLibraryPath (rootPath, usrPath, libraryPath);
#elif JUCE_MAC
    const String libraryPath = "Library/Application Support/ChowKick/tuning_library";
    const auto rootPath = File ("/");
    const auto usrPath = File::getSpecialLocation (File::userHomeDirectory);
    return getLibraryPath (rootPath, usrPath, libraryPath);
#elif JUCE_LINUX
    const String libraryPath = "usr/share/ChowKick/tuning_library";
    const auto rootPath = File ("/");
    const auto usrPath = File::getSpecialLocation (File::userHomeDirectory);
    return getLibraryPath (rootPath, usrPath, libraryPath);
#else
    return File();
#endif
}
} // namespace

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
    // constexpr auto fileChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
    auto* rootMenu = getRootMenu();
    rootMenu->clear();

    auto factoryTuningPath = getFactoryTuningLibrary();

    auto sclName = trigger.getScaleName();
    auto scaleOption = "Select SCL" + (sclName.isEmpty() ? "" : " (" + sclName + ")");
    rootMenu->addItem (scaleOption, [=]
                       {
                           resetMenuText();
                           FileChooser chooser ("Choose Scale", factoryTuningPath, "*.scl");
                           if (chooser.browseForFileToOpen())
                               trigger.setScaleFile (chooser.getResult());
                       });

    auto kbmName = trigger.getMappingName();
    auto mappingOption = "Select KBM" + (kbmName.isEmpty() ? "" : " (" + kbmName + ")");
    rootMenu->addItem (mappingOption, [=]
                       {
                           resetMenuText();
                           FileChooser chooser ("Choose Keyboard Mapping", factoryTuningPath, "*.kbm");
                           if (chooser.browseForFileToOpen())
                               trigger.setMappingFile (chooser.getResult());
                       });

    rootMenu->addItem ("Reset to Standard (12TET)", [=]
                       { trigger.resetTuning(); });

    rootMenu->addSeparator();
    if (factoryTuningPath != File())
    {
        rootMenu->addItem ("Open Factory Tuning Directory", [=]
                           { factoryTuningPath.startAsProcess(); });
    }

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
