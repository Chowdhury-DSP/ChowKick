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
}

std::unique_ptr<XmlElement> PresetManager::savePresetState()
{
    auto state = vts.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());

    auto tuningXml = std::make_unique<XmlElement> ("tuning_data");
    plugin->getTrigger().getTuningState (tuningXml.get());
    xml->addChildElement (tuningXml.release());

    return std::move (xml);
}

void PresetManager::loadPresetState (const XmlElement* xml)
{
    if (auto* tuningXml = xml->getChildByName ("tuning_data"))
        plugin->getTrigger().setTuningState (tuningXml);
    else
        plugin->getTrigger().resetTuning();

    vts.replaceState (ValueTree::fromXml (*xml));
}
