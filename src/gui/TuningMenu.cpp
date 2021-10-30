#include "TuningMenu.h"

namespace
{
const String userLibraryPath = "ChowdhuryDSP/ChowKick/UserTuningLibrary.txt";

File getFactoryTuningLibrary()
{
    [[maybe_unused]] auto getLibraryPath = [] (const File& rootPath, const File& usrPath, const String& libPath)
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
#elif JUCE_IOS
    const String libraryPath = "ExtraContent/tuning_library";
    const auto usrPath = File::getSpecialLocation (File::currentApplicationFile);
    return usrPath.getChildFile (libraryPath);
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
    
#if JUCE_IOS
    // download tuning library content...
    auto factoryTuningPath = getFactoryTuningLibrary();
    if (! factoryTuningPath.isDirectory())
    {
        factoryTuningPath.deleteFile();
        factoryTuningPath.createDirectory();
        
        URL libraryUrl { "https://ccrma.stanford.edu/~jatin/chowdsp/tuning_library.zip" };
        downloadTask = libraryUrl.downloadToFile (factoryTuningPath.getSiblingFile ("tuning_library.zip"), {}, this);
    }
#endif
}

TuningMenu::~TuningMenu()
{
    trigger.removeListener (this);
}

void TuningMenu::finished (URL::DownloadTask* task, bool success)
{
    auto factoryTuningPath = getFactoryTuningLibrary();
    if (! success)
    {
        factoryTuningPath.deleteRecursively();
        return;
    }
    
    auto downloadFile = task->getTargetLocation();
    ZipFile zipFile (downloadFile);
    auto result = zipFile.uncompressTo (factoryTuningPath);
    
    if (result.failed())
        factoryTuningPath.deleteRecursively();
    
    downloadFile.deleteFile();
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

#if ! JUCE_IOS
    rootMenu->addItem ("Select user tuning directory", [=]
                       {
                           resetMenuText();
                           chooseUserTuningLibraryPath (fileChooser, [=]
                                                        { refreshMenu(); });
                       });
#endif

    rootMenu->addSeparator();
    if (factoryTuningPath != File() && factoryTuningPath.isDirectory())
    {
        rootMenu->addItem ("Open Factory Tuning Directory", [=]
                           {
                               resetMenuText();
                               auto success = factoryTuningPath.startAsProcess();
                               jassert (success);
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

void TuningMenu::resetMenuText()
{
    MessageManagerLock mml;
    setText ("Tuning", dontSendNotification);
}

void TuningMenu::tuningLoadError (const String& message)
{
#if JUCE_IOS
    NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon, "Tuning Error", message);
#else
    AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Tuning Error", message);
#endif
}
