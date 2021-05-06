#pragma once

/**
 * Pre-compiled headers for JUCE plugins
 */

// C++/STL headers here...

// JUCE modules
#include <JuceHeader.h>

// Any other widely used headers that don't change...

// global definitions
using Parameters = std::vector<std::unique_ptr<RangedAudioParameter>>;
