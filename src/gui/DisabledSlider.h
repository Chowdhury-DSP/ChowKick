#pragma once

#include "ChowKick.h"

class DisabledSlider : public foleys::GuiItem,
                       private AudioProcessorValueTreeState::Listener
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (DisabledSlider)

    static const juce::Identifier pSliderType;
    static const juce::StringArray pSliderTypes;

    static const juce::Identifier pSliderTextBox;
    static const juce::StringArray pTextBoxPositions;

    static const juce::Identifier pValue;
    static const juce::Identifier pMinValue;
    static const juce::Identifier pMaxValue;

    static const juce::Identifier pDisableParam;
    static const juce::Identifier pDisableForVal;

    DisabledSlider (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);
    ~DisabledSlider() override;

    void parameterChanged (const String& paramID, float newValue) override;

    void update() override;
    std::vector<foleys::SettableProperty> getSettableProperties() const override;

    juce::String getControlledParameterID (juce::Point<int>) override;
    juce::Component* getWrappedComponent() override;

private:
    foleys::AutoOrientationSlider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    juce::String paramListening = {};
    AudioProcessorValueTreeState& vts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisabledSlider)
};
