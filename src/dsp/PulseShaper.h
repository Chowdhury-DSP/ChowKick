#pragma once

#include <pch.h>

class PulseShaper
{
public:
    PulseShaper (AudioProcessorValueTreeState& vts);

    static void addParameters (Parameters& params);
    void reset (double sampleRate);
    void processBlock (float* block, const int numSamples);

    inline float processSample (float x) noexcept
    {
        Vs.setVoltage (x);

        d53.incident (P2->reflected());
        double y = r162.voltage();
        P2->incident (d53.reflected());

        return (float) y;
    }

private:
    std::atomic<float>* r1Param = nullptr;
    std::atomic<float>* r2Param = nullptr;

    chowdsp::WDF::ResistiveVoltageSource Vs;

    chowdsp::WDF::Resistor r162 {   4700.0 };
    chowdsp::WDF::Resistor r163 { 100000.0 };
    std::unique_ptr<chowdsp::WDF::Capacitor> c40;
    chowdsp::WDF::Diode d53 { 2.52e-9, 25.85e-3 }; // 1N4148 diode
    
    std::unique_ptr<chowdsp::WDF::PolarityInverter> I1;
    std::unique_ptr<chowdsp::WDF::WDFSeries> S1;
    std::unique_ptr<chowdsp::WDF::WDFParallel> P1;
    std::unique_ptr<chowdsp::WDF::WDFParallel> P2;
};
