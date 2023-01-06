#pragma once

namespace VersionHints
{
using namespace chowdsp::version_literals;

/** Parameters with this version hint existed in the plugin before the plugin started using version hints. */
constexpr int original = 0;

/** Parameters introduced in version 1.2.0. */
constexpr int v1_2_0 = "1.2.0"_v.getVersionHint();
} // namespace VersionHints
