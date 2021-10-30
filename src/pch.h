#pragma once

/**
 * Pre-compiled headers for JUCE plugins
 */

// C++/STL headers here...

// JUCE modules
#include <JuceHeader.h>

// Any other widely used headers that don't change...
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4244)
#include <Tunings.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

// global definitions
using Parameters = std::vector<std::unique_ptr<RangedAudioParameter>>;
using Vec = dsp::SIMDRegister<float>;
