#include "DisabledSlider.h"

DisabledSlider::DisabledSlider (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node),
                                                                                                 vts (dynamic_cast<foleys::MagicProcessorState*> (&builder.getMagicState())->getValueTreeState())
{
    setColourTranslation (
        { { "slider-background", juce::Slider::backgroundColourId },
          { "slider-thumb", juce::Slider::thumbColourId },
          { "slider-track", juce::Slider::trackColourId },
          { "rotary-fill", juce::Slider::rotarySliderFillColourId },
          { "rotary-outline", juce::Slider::rotarySliderOutlineColourId },
          { "slider-text", juce::Slider::textBoxTextColourId },
          { "slider-text-background", juce::Slider::textBoxBackgroundColourId },
          { "slider-text-highlight", juce::Slider::textBoxHighlightColourId },
          { "slider-text-outline", juce::Slider::textBoxOutlineColourId } });

    addAndMakeVisible (slider);
}

DisabledSlider::~DisabledSlider()
{
    if (paramListening.isNotEmpty())
        vts.removeParameterListener (paramListening, this);
}

void DisabledSlider::parameterChanged (const String& paramID, float newValue)
{
    if (paramID != paramListening)
        return;

    bool parameterOn = newValue > 0.5f;
    auto disableWhenOn = (bool) getProperty (pDisableWhenOn);

    slider.setEnabled (parameterOn != disableWhenOn);
}

void DisabledSlider::update()
{
    attachment.reset();

    auto type = getProperty (pSliderType).toString();
    slider.setAutoOrientation (type.isEmpty() || type == pSliderTypes[0]);

    if (type == pSliderTypes[1])
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
    else if (type == pSliderTypes[2])
        slider.setSliderStyle (juce::Slider::LinearVertical);
    else if (type == pSliderTypes[3])
        slider.setSliderStyle (juce::Slider::Rotary);
    else if (type == pSliderTypes[4])
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    else if (type == pSliderTypes[5])
        slider.setSliderStyle (juce::Slider::IncDecButtons);

    auto textbox = getProperty (pSliderTextBox).toString();
    if (textbox == pTextBoxPositions[0])
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, slider.getTextBoxWidth(), slider.getTextBoxHeight());
    else if (textbox == pTextBoxPositions[1])
        slider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, slider.getTextBoxWidth(), slider.getTextBoxHeight());
    else if (textbox == pTextBoxPositions[3])
        slider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, slider.getTextBoxWidth(), slider.getTextBoxHeight());
    else if (textbox == pTextBoxPositions[4])
        slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, slider.getTextBoxWidth(), slider.getTextBoxHeight());
    else
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, slider.getTextBoxWidth(), slider.getTextBoxHeight());

    double minValue = getProperty (pMinValue);
    double maxValue = getProperty (pMaxValue);
    if (maxValue > minValue)
        slider.setRange (minValue, maxValue);

    auto valueID = configNode.getProperty (pValue, juce::String()).toString();
    if (valueID.isNotEmpty())
        slider.getValueObject().referTo (getMagicState().getPropertyAsValue (valueID));

    auto paramID = getControlledParameterID ({});
    if (paramID.isNotEmpty())
        attachment = getMagicState().createAttachment (paramID, slider);

    auto newParamListening = configNode.getProperty (pDisableParam, juce::String()).toString();
    if (newParamListening != paramListening)
    {
        if (paramListening.isNotEmpty())
            vts.removeParameterListener (paramListening, this);

        vts.addParameterListener (newParamListening, this);
        paramListening = newParamListening;
    }
}

std::vector<foleys::SettableProperty> DisabledSlider::getSettableProperties() const
{
    using namespace foleys;
    std::vector<SettableProperty> props;

    props.push_back ({ configNode, IDs::parameter, SettableProperty::Choice, {}, magicBuilder.createParameterMenuLambda() });
    props.push_back ({ configNode, pSliderType, SettableProperty::Choice, pSliderTypes[0], magicBuilder.createChoicesMenuLambda (pSliderTypes) });
    props.push_back ({ configNode, pSliderTextBox, SettableProperty::Choice, pTextBoxPositions[2], magicBuilder.createChoicesMenuLambda (pTextBoxPositions) });
    props.push_back ({ configNode, pValue, SettableProperty::Choice, 1.0f, magicBuilder.createPropertiesMenuLambda() });
    props.push_back ({ configNode, pMinValue, SettableProperty::Number, 0.0f, {} });
    props.push_back ({ configNode, pMaxValue, SettableProperty::Number, 2.0f, {} });
    props.push_back ({ configNode, pDisableParam, SettableProperty::Choice, {}, magicBuilder.createParameterMenuLambda() });
    props.push_back ({ configNode, pDisableWhenOn, SettableProperty::Toggle, {}, {} });

    return props;
}

juce::String DisabledSlider::getControlledParameterID (juce::Point<int>)
{
    return configNode.getProperty (foleys::IDs::parameter, juce::String()).toString();
}

juce::Component* DisabledSlider::getWrappedComponent()
{
    return &slider;
}

const juce::Identifier DisabledSlider::pSliderType { "slider-type" };
const juce::StringArray DisabledSlider::pSliderTypes { "auto", "linear-horizontal", "linear-vertical", "rotary", "rotary-horizontal-vertical", "inc-dec-buttons" };
const juce::Identifier DisabledSlider::pSliderTextBox { "slider-textbox" };
const juce::StringArray DisabledSlider::pTextBoxPositions { "no-textbox", "textbox-above", "textbox-below", "textbox-left", "textbox-right" };
const juce::Identifier DisabledSlider::pValue { "value" };
const juce::Identifier DisabledSlider::pMinValue { "min-value" };
const juce::Identifier DisabledSlider::pMaxValue { "max-value" };
const juce::Identifier DisabledSlider::pDisableParam { "disable-param" };
const juce::Identifier DisabledSlider::pDisableWhenOn { "disable-when-on" };
