#pragma once

#include <pch.h>

class ChowKick;
class PresetManager : public chowdsp::PresetManager
{
public:
    PresetManager (AudioProcessorValueTreeState& vts);

    chowdsp::Preset loadUserPresetFromFile (const File& file) override;

private:
    ChowKick* plugin;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
