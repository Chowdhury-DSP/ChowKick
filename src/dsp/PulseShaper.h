#pragma once

#include <pch.h>

namespace ShaperTags
{
const String sustainTag = "pulse_sustain";
const String decayTag = "pulse_decay";
} // namespace ShaperTags

class PulseShaper
{
public:
    PulseShaper (AudioProcessorValueTreeState& vts, double sampleRate);

    static void addParameters (Parameters& params);
    void processBlock (dsp::AudioBlock<Vec>& block, int numSamples);

    inline Vec processSample (Vec x) noexcept
    {
        Vs.setVoltage (x);

        d53.incident (P2.reflected());
        Vec y = chowdsp::WDFT::voltage<Vec> (r162);
        P2.incident (d53.reflected());

        return y;
    }

private:
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* decayParam = nullptr;

    using Resistor = chowdsp::WDFT::ResistorT<Vec>;
    using Capacitor = chowdsp::WDFT::CapacitorAlphaT<Vec>;
    using ResVs = chowdsp::WDFT::ResistiveVoltageSourceT<Vec>;

    ResVs Vs;
    Resistor r162 { 4700.0f };
    Resistor r163 { 100000.0f };
    Capacitor c40;

    chowdsp::WDFT::WDFParallelT<Vec, Capacitor, Resistor> P1 { c40, r163 };
    chowdsp::WDFT::WDFSeriesT<Vec, ResVs, decltype (P1)> S1 { Vs, P1 };
    chowdsp::WDFT::PolarityInverterT<Vec, Resistor> I1 { r162 };
    chowdsp::WDFT::WDFParallelT<Vec, decltype (I1), decltype (S1)> P2 { I1, S1 };

    chowdsp::WDFT::DiodeT<Vec, decltype (P2)> d53 { P2, 2.52e-9f }; // 1N4148 diode
};
