#include "PresetManager.h"
#include "ChowKick.h"

namespace
{
const String userPresetPath = "ChowdhuryDSP/ChowKick/UserPresets.txt";
}

PresetManager::PresetManager (AudioProcessorValueTreeState& vtState) : chowdsp::PresetManager (vtState)
{
    plugin = dynamic_cast<ChowKick*> (&vts.processor);

    setUserPresetConfigFile (userPresetPath);
    setDefaultPreset (chowdsp::Preset { BinaryData::Default_chowpreset, BinaryData::Default_chowpresetSize });

    std::vector<chowdsp::Preset> factoryPresets;
    factoryPresets.emplace_back (BinaryData::Bouncy_chowpreset, BinaryData::Bouncy_chowpresetSize);
    factoryPresets.emplace_back (BinaryData::Tight_chowpreset, BinaryData::Tight_chowpresetSize);
    factoryPresets.emplace_back (BinaryData::Tonal_chowpreset, BinaryData::Tonal_chowpresetSize);
    factoryPresets.emplace_back (BinaryData::Wonky_Synth_chowpreset, BinaryData::Wonky_Synth_chowpresetSize);
    addPresets (factoryPresets);

    loadDefaultPreset();

#if JUCE_IOS
    File appDataDir = File::getSpecialLocation (File::userApplicationDataDirectory);
    auto userPresetFolder = appDataDir.getChildFile (userPresetPath).getSiblingFile ("Presets");
    if (! userPresetFolder.isDirectory())
    {
        userPresetFolder.deleteFile();
        userPresetFolder.createDirectory();
    }

    setUserPresetPath (userPresetFolder);
#endif // JUCE_IOS
}

chowdsp::Preset PresetManager::loadUserPresetFromFile (const File& file)
{
    chowdsp::Preset compatiblePreset { file };
    if (compatiblePreset.isValid())
        return std::move (compatiblePreset);

    auto xml = XmlDocument::parse (file);
    if (xml == nullptr)
        return compatiblePreset;

    if (xml->getTagName() != chowdsp::Preset::presetTag.toString())
        return compatiblePreset;

    auto name = xml->getStringAttribute (chowdsp::Preset::nameTag);
    if (name.isEmpty())
        return compatiblePreset;

    if (xml->getStringAttribute (chowdsp::Preset::pluginTag) != JucePlugin_Name)
        return compatiblePreset;

    auto vendor = xml->getStringAttribute (chowdsp::Preset::vendorTag);
    if (vendor.isEmpty())
    {
        vendor = name.upToFirstOccurrenceOf ("_", false, false);
        name = name.fromLastOccurrenceOf ("_", false, false);
    }

    auto category = xml->getStringAttribute (chowdsp::Preset::categoryTag);

    auto* xmlState = xml->getChildElement (0);
    if (xmlState == nullptr)
        return compatiblePreset;

    return { name, vendor, *xmlState, category };
}
