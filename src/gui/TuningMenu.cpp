#include "TuningMenu.h"

namespace
{
const String userLibraryPath = "ChowdhuryDSP/ChowKick/UserTuningLibrary.txt";

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

File getUserLibraryConfigFile()
{
    File updatePresetFile = File::getSpecialLocation (File::userApplicationDataDirectory);
    return updatePresetFile.getChildFile (userLibraryPath);
}

template <typename Callback>
void chooseUserTuningLibraryPath (std::shared_ptr<FileChooser>& fileChooser, Callback&& callback)
{
    constexpr auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
    fileChooser = std::make_shared<FileChooser> ("Choose tuning library folder");
    fileChooser->launchAsync (flags, [=] (const FileChooser& fc)
                              {
                                  auto result = fc.getResult();
                                  auto config = getUserLibraryConfigFile();
                                  config.deleteFile();
                                  config.create();
                                  config.replaceWithText (result.getFullPathName());
                                  callback();
                              });
}

File getUserTuningLibraryPath()
{
    auto config = getUserLibraryConfigFile();
    if (config.existsAsFile())
        return File (config.loadFileAsString());

    return File();
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
    constexpr auto fileChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
    auto* rootMenu = getRootMenu();
    rootMenu->clear();

    auto userTuningPath = getUserTuningLibraryPath();
    auto factoryTuningPath = getFactoryTuningLibrary();
    auto defaultTuningPath = userTuningPath != File() ? userTuningPath : factoryTuningPath;

    auto sclName = trigger.getScaleName();
    auto scaleOption = "Select SCL" + (sclName.isEmpty() ? "" : " (" + sclName + ")");
    rootMenu->addItem (scaleOption, [=]
                       {
                           resetMenuText();
                           fileChooser = std::make_shared<FileChooser> ("Choose Scale", factoryTuningPath, "*.scl");
                           fileChooser->launchAsync (fileChooserFlags, [=] (const FileChooser& fc)
                                                     { trigger.setScaleFile (fc.getResult()); });
                       });

    auto kbmName = trigger.getMappingName();
    auto mappingOption = "Select KBM" + (kbmName.isEmpty() ? "" : " (" + kbmName + ")");
    rootMenu->addItem (mappingOption, [=]
                       {
                           resetMenuText();
                           fileChooser = std::make_shared<FileChooser> ("Choose Keyboard Mapping", factoryTuningPath, "*.kbm");
                           fileChooser->launchAsync (fileChooserFlags, [=] (const FileChooser& fc)
                                                     { trigger.setMappingFile (fc.getResult()); });
                       });

    rootMenu->addItem ("Reset to Standard (12TET)", [=]
                       { trigger.resetTuning(); });

    rootMenu->addItem ("Select user tuning directory", [=]
                       {
                           resetMenuText();
                           chooseUserTuningLibraryPath (fileChooser, [=]
                                                        { refreshMenu(); });
                       });

    rootMenu->addSeparator();
    if (factoryTuningPath != File())
    {
        rootMenu->addItem ("Open Factory Tuning Directory", [=]
                           {
                               resetMenuText();
                               factoryTuningPath.startAsProcess();
                           });
    }
    if (userTuningPath != File())
    {
        rootMenu->addItem ("Open User Tuning Directory", [=]
                           {
                               resetMenuText();
                               userTuningPath.startAsProcess();
                           });
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
