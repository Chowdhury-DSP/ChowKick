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
