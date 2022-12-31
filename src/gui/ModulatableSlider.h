#pragma once

#include <pch.h>

class ModulatableSlider : public Slider,
                          private Timer
{
public:
    explicit ModulatableSlider (const chowdsp::HostContextProvider& hostContextProvider);

    void paint (Graphics& g) override;
    void mouseDown (const MouseEvent& e) override;
    void attachToParameter (chowdsp::FloatParameter* newParam);

    std::unique_ptr<SliderParameterAttachment> attachment;

protected:
    void timerCallback() override;
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float modSliderPos);
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float modSliderPos);

    const chowdsp::FloatParameter* param = nullptr;
    const chowdsp::HostContextProvider& hostContextProvider;

    double modulatedValue = 0.0;

    struct KnobAssets
    {
        std::unique_ptr<Drawable> knob = Drawable::createFromImageData (chowdsp_BinaryData::knob_svg, chowdsp_BinaryData::knob_svgSize);
        std::unique_ptr<Drawable> pointer = Drawable::createFromImageData (chowdsp_BinaryData::pointer_svg, chowdsp_BinaryData::pointer_svgSize);
    };
    SharedResourcePointer<KnobAssets> sharedAssets;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulatableSlider)
};

//====================================================================
class ModSliderItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (ModSliderItem)

    static const juce::Identifier pSliderType;
    static const juce::StringArray pSliderTypes;

    static const juce::Identifier pSliderTextBox;
    static const juce::StringArray pTextBoxPositions;
    static const juce::Identifier pSliderTextBoxWidth;
    static const juce::Identifier pSliderTextBoxHeight;

    static const juce::Identifier pValue;
    static const juce::Identifier pMinValue;
    static const juce::Identifier pMaxValue;

    static const juce::Identifier pDisableParam;
    static const juce::Identifier pDisableForVal;

    ModSliderItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);

    void update() override;
    void resized() override;

    std::vector<foleys::SettableProperty> getSettableProperties() const override;
    juce::String getControlledParameterID (juce::Point<int>) override;
    juce::Component* getWrappedComponent() override;

    void mouseDrag (const juce::MouseEvent& e) override;

private:
    std::optional<ModulatableSlider> slider;

    int defaultHeight = 0;
    int sliderTextBoxHeight = 0;
    juce::Slider::TextEntryBoxPosition textBoxPosition;

    std::optional<juce::ParameterAttachment> disableAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModSliderItem)
};
