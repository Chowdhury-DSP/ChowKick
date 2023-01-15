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
    const String libraryPath = "ChowdhuryDSP/ChowKick/tuning_library";
    const auto usrPath = File::getSpecialLocation (File::userApplicationDataDirectory);
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
                                  callback(); });
}

[[maybe_unused]] File getUserTuningLibraryPath()
{
    auto config = getUserLibraryConfigFile();
    if (config.existsAsFile())
        return { config.loadFileAsString() };

    return {};
}
} // namespace

TuningMenu::TuningMenu (Trigger& trig, AudioProcessorValueTreeState& vts)
    : trigger (trig), useMTSParam (*vts.getParameter (TriggerTags::useMTSTag.getParamID()))
{
    trigger.addListener (this);
    wasMTSParamOn = useMTSParam.getValue() > 0.5f;
    wasMTSMasterConnected = trigger.isMTSAvailable();
    refreshMenu();

    startTimer (100);

    setColour (ComboBox::backgroundColourId, Colours::transparentBlack);
    setJustificationType (Justification::centred);

#if JUCE_IOS
    // unpack tuning library content...
    auto factoryTuningPath = getFactoryTuningLibrary();
    if (! factoryTuningPath.isDirectory())
    {
        factoryTuningPath.deleteFile();
        factoryTuningPath.createDirectory();

        MemoryInputStream tuningInStream (BinaryData::tuning_library_zip, BinaryData::tuning_library_zipSize, false);
        ZipFile tuningZip (tuningInStream);
        auto unzipResult = tuningZip.uncompressTo (factoryTuningPath);
        if (unzipResult.failed())
        {
            factoryTuningPath.deleteRecursively();
            DBG ("Unzipping tuning library failed! Error: " + unzipResult.getErrorMessage());
            return;
        }
    }
#endif
}

TuningMenu::~TuningMenu()
{
    trigger.removeListener (this);
}

void TuningMenu::timerCallback()
{
    const auto isUsingMTSParamOn = useMTSParam.getValue() > 0.5f;
    const auto isMTSMasterConnected = trigger.isMTSAvailable();

    if (isMTSMasterConnected != wasMTSMasterConnected || isUsingMTSParamOn != wasMTSParamOn)
    {
        wasMTSParamOn = isUsingMTSParamOn;
        wasMTSMasterConnected = isMTSMasterConnected;
        refreshMenu();
    }
}

void TuningMenu::addMTSOptionToMenu (PopupMenu& menu)
{
    if (wasMTSMasterConnected)
    {
        PopupMenu::Item mtsItem;
        mtsItem.itemID = 1001;
        mtsItem.text = "Use MTS Tuning";
        mtsItem.colour = wasMTSParamOn ? Colour { 0xFFFFB200 } : Colours::white;
        mtsItem.action = [this]
        {
            useMTSParam.setValueNotifyingHost ((useMTSParam.getValue() > 0.5f) ? 0.0f : 1.0f);
            refreshMenu();
        };
        menu.addItem (mtsItem);
    }
}

void TuningMenu::refreshMenu()
{
#if JUCE_IOS
    refreshMenuIOS();
#else
    constexpr auto fileChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
    auto* rootMenu = getRootMenu();
    rootMenu->clear();

    const auto userTuningPath = getUserTuningLibraryPath();
    const auto factoryTuningPath = getFactoryTuningLibrary();
    const auto defaultTuningPath = userTuningPath != File() ? userTuningPath : factoryTuningPath;

    auto sclName = trigger.getScaleName();
    auto scaleOption = "Select SCL" + (sclName.isEmpty() ? "" : " (" + sclName + ")");
    rootMenu->addItem (scaleOption, [this, factoryTuningPath, fileChooserFlags]
                       {
                           ignoreUnused (fileChooserFlags);
                           resetMenuText();
                           fileChooser = std::make_shared<FileChooser> ("Choose Scale", factoryTuningPath, "*.scl");
                           fileChooser->launchAsync (fileChooserFlags, [this] (const FileChooser& fc)
                                                     { trigger.setScaleFile (fc.getResult()); }); });

    auto kbmName = trigger.getMappingName();
    auto mappingOption = "Select KBM" + (kbmName.isEmpty() ? "" : " (" + kbmName + ")");
    rootMenu->addItem (mappingOption, [this, factoryTuningPath, fileChooserFlags]
                       {
                           ignoreUnused (fileChooserFlags);
                           resetMenuText();
                           fileChooser = std::make_shared<FileChooser> ("Choose Keyboard Mapping", factoryTuningPath, "*.kbm");
                           fileChooser->launchAsync (fileChooserFlags, [this] (const FileChooser& fc)
                                                     { trigger.setMappingFile (fc.getResult()); }); });

    rootMenu->addItem ("Reset to Standard (12TET)", [this]
                       { trigger.resetTuning(); });

    addMTSOptionToMenu (*rootMenu);

    rootMenu->addSeparator();
    rootMenu->addItem ("Select user tuning directory", [this]
                       {
                           resetMenuText();
                           chooseUserTuningLibraryPath (fileChooser, [this]
                                                        { refreshMenu(); }); });

    if (factoryTuningPath != File() && factoryTuningPath.isDirectory())
    {
        rootMenu->addItem ("Open Factory Tuning Directory", [this, factoryTuningPath]
                           {
                               resetMenuText();
                               factoryTuningPath.startAsProcess(); });
    }
    if (userTuningPath != File())
    {
        rootMenu->addItem ("Open User Tuning Directory", [this, userTuningPath]
                           {
                               resetMenuText();
                               userTuningPath.startAsProcess(); });
    }

    resetMenuText();
#endif
}

#if JUCE_IOS
void TuningMenu::refreshMenuIOS()
{
    auto* rootMenu = getRootMenu();
    rootMenu->clear();

    const auto factoryTuningPath = getFactoryTuningLibrary();

    auto sclName = trigger.getScaleName();
    auto scaleOption = "Select SCL" + (sclName.isEmpty() ? "" : " (" + sclName + ")");
    PopupMenu scaleMenu;
    {
        std::map<String, PopupMenu> menuMap;
        PopupMenu otherMenu;

        auto getMapForSCLFile = [&] (const String& filePath) -> PopupMenu&
        {
            auto getMenu = [&menuMap] (const String& key) -> PopupMenu&
            {
                return menuMap.emplace (std::make_pair (key, PopupMenu())).first->second;
            };

            if (filePath.contains ("Equal") && filePath.contains ("Temperament"))
                return getMenu ("Equal Temperament");
            if (filePath.contains ("ED2"))
                return getMenu ("Equal Division (harmonic 2)");
            if (filePath.contains ("ED3"))
                return getMenu ("Equal Division (harmonic 3)");
            if (filePath.contains ("ED4"))
                return getMenu ("Equal Division (harmonic 4");
            if (filePath.contains ("HD2"))
                return getMenu ("Harmonic Division (harmonic 2)");
            if (filePath.contains ("HD3"))
                return getMenu ("Harmonic Division (harmonic 3)");
            if (filePath.contains ("HD4"))
                return getMenu ("Harmonic Division (harmonic 4)");
            if (filePath.contains ("SD2"))
                return getMenu ("Subharmonic Division (harmonic 2)");
            if (filePath.contains ("SD3"))
                return getMenu ("Subharmonic Division (harmonic 3)");
            if (filePath.contains ("SD4"))
                return getMenu ("Subharmonic Division (harmonic 4)");

            return otherMenu;
        };

        for (DirectoryEntry entry : RangedDirectoryIterator (factoryTuningPath, true, "*.scl"))
        {
            const auto& scaleFile = entry.getFile();
            auto& subMenu = getMapForSCLFile (scaleFile.getFullPathName());
            subMenu.addItem (scaleFile.getFileName(), [=]
                             {
                                 resetMenuText();
                                 trigger.setScaleFile (scaleFile); });
        }

        for (auto& keyMenuPair : menuMap)
            scaleMenu.addSubMenu (keyMenuPair.first, keyMenuPair.second);

        PopupMenu::MenuItemIterator it (otherMenu);
        while (it.next())
            scaleMenu.addItem (it.getItem());
    }

    rootMenu->addSubMenu (scaleOption, scaleMenu);

    auto kbmName = trigger.getMappingName();
    auto mappingOption = "Select KBM" + (kbmName.isEmpty() ? "" : " (" + kbmName + ")");
    PopupMenu mappingMenu;
    for (DirectoryEntry entry : RangedDirectoryIterator (factoryTuningPath, true, "*.kbm"))
    {
        const auto& mappingFile = entry.getFile();
        if (! mappingFile.getFullPathName().contains ("KBM Concert Pitch"))
            continue;

        mappingMenu.addItem (mappingFile.getFileName(), [=]
                             {
                                 resetMenuText();
                                 trigger.setMappingFile (mappingFile); });
    }

    rootMenu->addSubMenu (mappingOption, mappingMenu);

    rootMenu->addItem ("Reset to Standard (12TET)", [this]
                       { trigger.resetTuning(); });

    resetMenuText();
}
#endif

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
