#pragma once

#include <pch.h>

class PulseShaper
{
public:
    PulseShaper (AudioProcessorValueTreeState& vts, double sampleRate);

    static void addParameters (Parameters& params);
    void processBlock (dsp::AudioBlock<Vec>& block, const int numSamples);

    inline Vec processSample (Vec x) noexcept
    {
        Vs.setVoltage (x);

        d53.incident (P2.reflected());
        Vec y = r162.voltage();
        P2.incident (d53.reflected());

        return y;
    }

private:
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* decayParam = nullptr;

    using Resistor = chowdsp::WDF::Resistor<Vec>;
    using Capacitor = chowdsp::WDF::Capacitor<Vec>;
    using ResVs = chowdsp::WDF::ResistiveVoltageSource<Vec>;

    ResVs Vs;
    Resistor r162 { 4700.0f };
    Resistor r163 { 100000.0f };
    Capacitor c40;
    chowdsp::WDF::Diode<Vec> d53 { 2.52e-9f, 25.85e-3f }; // 1N4148 diode

    using P1Type = chowdsp::WDF::WDFParallelT<Vec, Capacitor, Resistor>;
    P1Type P1 { c40, r163 };

    using S1Type = chowdsp::WDF::WDFSeriesT<Vec, ResVs, P1Type>;
    S1Type S1 { Vs, P1 };

    using I1Type = chowdsp::WDF::PolarityInverterT<Vec, Resistor>;
    I1Type I1 { r162 };

    using P2Type = chowdsp::WDF::WDFParallelT<Vec, I1Type, S1Type>;
    P2Type P2 { I1, S1 };
};
