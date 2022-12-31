#pragma once

#include <pch.h>

namespace ShaperTags
{
const juce::ParameterID sustainTag { "pulse_sustain", VersionHints::original };
const juce::ParameterID decayTag { "pulse_decay", VersionHints::original };
} // namespace ShaperTags

class PulseShaper
{
    using v_type = xsimd::batch<float>;

public:
    PulseShaper (AudioProcessorValueTreeState& vts, double sampleRate);

    static void addParameters (Parameters& params);
    void processBlock (chowdsp::AudioBlock<Vec>& block, int numSamples);

    inline Vec processSample (Vec x) noexcept
    {
        Vs.setVoltage (x);

        d53.incident (P2.reflected());
        const auto y = wdft::voltage<v_type> (r162);
        P2.incident (d53.reflected());

        return y;
    }

private:
    chowdsp::FloatParameter* sustainParam = nullptr;
    chowdsp::FloatParameter* decayParam = nullptr;

    using Resistor = wdft::ResistorT<v_type>;
    using Capacitor = wdft::CapacitorAlphaT<v_type>;
    using ResVs = wdft::ResistiveVoltageSourceT<v_type>;

    ResVs Vs;
    Resistor r162 { 4700.0f };
    Resistor r163 { 100000.0f };
    Capacitor c40;

    wdft::WDFParallelT<v_type, Capacitor, Resistor> P1 { c40, r163 };
    wdft::WDFSeriesT<v_type, ResVs, decltype (P1)> S1 { Vs, P1 };
    wdft::PolarityInverterT<v_type, Resistor> I1 { r162 };
    wdft::WDFParallelT<v_type, decltype (I1), decltype (S1)> P2 { I1, S1 };

    wdft::DiodeT<v_type, decltype (P2)> d53 { P2, 2.52e-9f }; // 1N4148 diode

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulseShaper)
};
