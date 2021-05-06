#pragma once

#include <pch.h>

class PulseShaper
{
public:
    PulseShaper (AudioProcessorValueTreeState& vts, double sampleRate);

    static void addParameters (Parameters& params);
    void reset (double sampleRate);
    void processBlock (float* block, const int numSamples);

    inline float processSample (float x) noexcept
    {
        Vs.setVoltage (x);

        d53.incident (P2.reflected());
        double y = r162.voltage();
        P2.incident (d53.reflected());

        return (float) y;
    }

private:
    std::atomic<float>* r1Param = nullptr;
    std::atomic<float>* r2Param = nullptr;

    using Resistor = chowdsp::WDF::Resistor;
    using Capacitor = chowdsp::WDF::Capacitor;
    using ResVs = chowdsp::WDF::ResistiveVoltageSource;


    ResVs Vs;
    Resistor r162 {   4700.0 };
    Resistor r163 { 100000.0 };
    Capacitor c40;
    chowdsp::WDF::Diode d53 { 2.52e-9, 25.85e-3 }; // 1N4148 diode
    
    using P1Type = chowdsp::WDF::WDFParallelT<Capacitor, Resistor>;
    P1Type P1 { c40, r163 };

    using S1Type = chowdsp::WDF::WDFSeriesT<ResVs,P1Type>;
    S1Type S1 { Vs, P1 };

    using I1Type = chowdsp::WDF::PolarityInverterT<Resistor>;
    I1Type I1 { r162 };

    using P2Type = chowdsp::WDF::WDFParallelT<I1Type, S1Type>;
    P2Type P2 { I1, S1 };
};
