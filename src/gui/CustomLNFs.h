#pragma once

#include <pch.h>

class SliderLNF : public chowdsp::ChowLNF
{
public:
    SliderLNF() = default;

    Label* createSliderTextBox (Slider& slider) override
    {
        auto l = chowdsp::ChowLNF::createSliderTextBox (slider);
        l->setFont ((float) slider.getTextBoxHeight() * 0.8f);

        return l;
    }
};

/** Custom LookAndFeel for Bottom Bar section */
class BottomBarLNF : public chowdsp::ChowLNF
{
public:
    BottomBarLNF()
    {
        setColour (PopupMenu::backgroundColourId, Colour (0xFF1B2A33));
        setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xFF425866));
    }

    ~BottomBarLNF() override = default;

    void drawPopupMenuItem (Graphics& g, const Rectangle<int>& area, const bool isSeparator, const bool isActive, const bool isHighlighted, const bool /*isTicked*/, const bool hasSubMenu, const String& text, const String& shortcutKeyText, const Drawable* icon, const Colour* const textColourToUse) override
    {
        LookAndFeel_V4::drawPopupMenuItem (g, area, isSeparator, isActive, isHighlighted, false /*isTicked*/, hasSubMenu, text, shortcutKeyText, icon, textColourToUse);
    }

    Component* getParentComponentForMenuOptions (const PopupMenu::Options& options) override
    {
#if JUCE_IOS
        if (PluginHostType::getPluginLoadedAs() == AudioProcessor::wrapperType_AudioUnitv3)
        {
            if (options.getParentComponent() == nullptr && options.getTargetComponent() != nullptr)
                return options.getTargetComponent()->getTopLevelComponent();
        }
#endif
        return LookAndFeel_V2::getParentComponentForMenuOptions (options);
    }

    void drawPopupMenuBackground (Graphics& g, int width, int height) override
    {
        g.fillAll (findColour (PopupMenu::backgroundColourId));
        ignoreUnused (width, height);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BottomBarLNF)
};

/** Custom LookAndFeel for ComboBoxes */
class ComboBoxLNF : public BottomBarLNF
{
public:
    ComboBoxLNF() = default;
    ~ComboBoxLNF() override = default;

    void drawComboBox (Graphics& g, int width, int height, bool, int, int, int, int, ComboBox& box) override
    {
        auto cornerSize = 5.0f;
        Rectangle<int> boxBounds (0, 0, width, height);

        g.setColour (box.findColour (ComboBox::backgroundColourId));
        g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

        if (box.getName().isNotEmpty())
        {
            g.setColour (Colours::white);
            g.setFont (getComboBoxFont (box).boldened());
            auto nameBox = boxBounds.withWidth (boxBounds.proportionOfWidth (0.7f));
            g.drawFittedText (box.getName() + ": ", nameBox, Justification::right, 1);
        }
    }

    void positionComboBoxText (ComboBox& box, Label& label) override
    {
        auto b = box.getBounds();

        if (box.getName().isNotEmpty())
        {
            auto width = b.proportionOfWidth (0.3f);
            auto x = b.proportionOfWidth (0.7f);
            b = b.withX (x).withWidth (width);
        }

        label.setBounds (b);
        label.setFont (getComboBoxFont (box).boldened());
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBoxLNF)
};
