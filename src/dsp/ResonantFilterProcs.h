#pragma once

#include <pch.h>

struct BasicFilterProc
{
    inline Vec operator() (Vec x, const Vec (&b)[3], const Vec (&a)[3],
                           Vec (&z)[3], float d1, float d2, float d3) const
    {
        auto y = z[1] + x * b[0];
        auto yDrive = drive (y, d3);
        z[1] = drive (z[2] + x * b[1] - yDrive * a[1], d1);
        z[2] = drive (x * b[2] - yDrive * a[2], d2);
        return y;
    }

    inline Vec drive (Vec x, float drive) const noexcept
    {
        using namespace chowdsp::SIMDUtils;
        return tanhSIMD (x * drive) / drive;
    }

    static inline float drive_deriv (float x)
    {
        return 1.0f - std::pow (std::tanh (x), 2.0f);
    }

    static auto getDriveValues (float tightParam, float bounceParam)
    {
        auto d1 = 4.9f * std::pow (tightParam, 4.0f) + 0.1f;
        auto d2 = 4.9f * std::pow (tightParam, 6.0f) + 0.1f;
        auto d3 = 4.75f * std::pow (bounceParam, 3.0f) + 0.25f;
        return std::make_tuple (d1, d2, d3);
    };

    static auto getNLFilterCorrections (float tightParam, float bounceParam, float b1, float b2, float a1, float a2, float T)
    {
        auto [d1, d2, d3] = getDriveValues (tightParam, bounceParam);
        auto g1 = drive_deriv (d1);
        auto g2 = drive_deriv (d2);
        auto g3 = drive_deriv (d3);

        auto b0Corr = -b1 - b2 + g1 * b1 + g1 * g2 * b2 ;
        auto b1Corr = (b2 - g1 * g2 * b2) * T;
        auto b2Corr = (T / 4.0f) * (g1 * g2 * b2 - g1 * b1 - b2 + b1);

        auto a0Corr = -a1 - a2 + g1 * g3 * a1 + g1 * g2 * g3 * a2;
        auto a1Corr = (a2 - g1 * g2 * g3 * a2) * T;
        auto a2Corr = (T / 4.0f) * (g1 * g2 * g3 * a2 - g1 * g3 * a1 - a2 + a1);

        return std::make_tuple (b0Corr, b1Corr, b2Corr, a0Corr, a1Corr, a2Corr);
    }
};

struct LinearFilterProc
{
    inline Vec operator() (Vec x, const Vec (&b)[3], const Vec (&a)[3],
                           Vec (&z)[3], float /*d1*/, float /*d2*/, float /*d3*/) const
    {
        auto y = (z[1] + x * b[0]) * 0.999999f;
        z[1] = z[2] + x * b[1] - y * a[1];
        z[2] = x * b[2] - y * a[2];
        return y * 0.15f;
    }

    static auto getDriveValues (float /*tightParam*/, float /*bounceParam*/)
    {
        return std::make_tuple (1.0f, 1.0f, 1.0f);
    };

    static auto getNLFilterCorrections (float, float, float, float, float, float, float)
    {
        return std::make_tuple (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
};

struct BouncyFilterProc
{
    inline Vec operator() (Vec x, const Vec (&b)[3], const Vec (&a)[3],
                           Vec (&z)[3], float d1, float d2, float d3) const
    {
        auto y = z[1] + x * b[0];
        auto yDrive = drive (y, d3);
        z[1] = drive (z[2] + x * b[1] - yDrive * a[1], d1);
        z[2] = drive (x * b[2] - y * a[2], d1);
        return y * d2;
    }

    inline Vec drive (Vec x, float drive) const noexcept
    {
        using namespace chowdsp::SIMDUtils;
        return tanhSIMD (x * drive) / drive;
    }

    static inline float drive_deriv (float x)
    {
        return 1.0f - std::pow (std::tanh (x), 2.0f);
    }

    static auto getDriveValues (float tightParam, float bounceParam)
    {
        auto d1 = 4.9f * std::pow (tightParam, 4.0f) + 0.1f;
        
        auto bounceScale = 0.7f * tightParam + 0.3f;
        auto d3 = std::pow (bounceScale * bounceParam, 2.0f) + 0.1f;

        auto d2 = 0.4f * std::pow (tightParam, 0.8f) + 0.4f * std::pow (1.0f - bounceParam, 0.8f) + 0.1f;

        return std::make_tuple (d1, d2, d3);
    };

    static auto getNLFilterCorrections (float tightParam, float bounceParam, float b1, float b2, float a1, float a2, float T)
    {
        auto [d1, d2, d3] = getDriveValues (tightParam, bounceParam);
        auto g1 = drive_deriv (d1);
        auto g2 = drive_deriv (d2);
        auto g3 = drive_deriv (d3);

        auto b0Corr = -b1 - b2 + g1 * b1 + g1 * g2 * b2 ;
        auto b1Corr = (b2 - g1 * g2 * b2) * T;
        auto b2Corr = (T / 4.0f) * (g1 * g2 * b2 - g1 * b1 - b2 + b1);

        auto a0Corr = -a1 - a2 + g1 * g3 * a1 + g1 * g2 * a2;
        auto a1Corr = (a2 - g1 * g2 * a2) * T;
        auto a2Corr = (T / 4.0f) * (g1 * g2 * a2 - g1 * g3 * a1 - a2 + a1);

        return std::make_tuple (b0Corr, b1Corr, b2Corr, a0Corr, a1Corr, a2Corr);
    }
};
