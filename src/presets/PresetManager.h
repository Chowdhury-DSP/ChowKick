#pragma once

#include <pch.h>

struct Preset
{
    Preset (String presetFile);
    Preset (const File& presetFile);
    void initialise (const ValueTree& parentTree);

    String name;
    ValueTree state;
    int index;
    bool isValid = false;
};

//====================================================
class PresetManager
{
public:
    PresetManager();

    StringArray getPresetChoices();
    void loadPresets();

    int getNumPresets() const { return presets.size(); }
    String getPresetName (int idx);
    bool setPreset (AudioProcessorValueTreeState& vts, int idx);

    void registerPresetsComponent (foleys::MagicGUIBuilder&);
    void presetUpdated() { listeners.call (&Listener::presetUpdated); }

    File getUserPresetFolder() { return userPresetFolder; }
    void chooseUserPresetFolder();
    bool saveUserPreset (const String& name, const AudioProcessorValueTreeState& vts);
    const PopupMenu& getUserPresetMenu() const { return userPresetMenu; }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void presetUpdated() {}
    };

    void addListener (Listener* l) { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

    constexpr static int maxNumPresets() { return 999; }

private:
    File getUserPresetConfigFile() const;
    void updateUserPresets();
    void loadPresetFolder (PopupMenu& menu, File& directory);
    File userPresetFolder;
    int numFactoryPresets = 0;
    PopupMenu userPresetMenu;

    HashMap<int, Preset*> presetMap;
    OwnedArray<Preset> presets;
    int maxIdx = 0;

    ListenerList<Listener> listeners;

    std::shared_ptr<FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
