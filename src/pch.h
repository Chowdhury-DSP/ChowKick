#pragma once

/**
 * Pre-compiled headers for JUCE plugins
 */

// C++/STL headers here...

// JUCE modules
#include <JuceHeader.h>

// Any other widely used headers that don't change...
#include <Tunings.h>

// global definitions
using Parameters = std::vector<std::unique_ptr<RangedAudioParameter>>;
using Vec = dsp::SIMDRegister<float>;
