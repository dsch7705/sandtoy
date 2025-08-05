#pragma once

#include <cstdint>
#include <cmath>
#include <functional>
#include <algorithm>
#include <string>


namespace Util
{
    uint32_t blendRGBA(uint32_t a, uint32_t b);

    inline uint32_t lerpColor(uint32_t c1, uint32_t c2, float t)
    {
        // Clamp t to [0, 1]
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        // Extract RGBA components
        uint8_t r1 = (c1 >> 24) & 0xFF;
        uint8_t g1 = (c1 >> 16) & 0xFF;
        uint8_t b1 = (c1 >> 8)  & 0xFF;
        uint8_t a1 =  c1        & 0xFF;

        uint8_t r2 = (c2 >> 24) & 0xFF;
        uint8_t g2 = (c2 >> 16) & 0xFF;
        uint8_t b2 = (c2 >> 8)  & 0xFF;
        uint8_t a2 =  c2        & 0xFF;

        // Linear interpolation
        uint8_t r = r1 + (r2 - r1) * t;
        uint8_t g = g1 + (g2 - g1) * t;
        uint8_t b = b1 + (b2 - b1) * t;
        uint8_t a = a1 + (a2 - a1) * t;

        // Recombine
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    constexpr float kAbsZero { -273.15f };
    constexpr float kMaxTemp { 3000.f };
    #define TEMP_COLOR_MODE_LIST \
        X(Normal) \
        X(Infrared) \

    enum class TemperatureColorMode
    {
    #define X(V) V,
        TEMP_COLOR_MODE_LIST
    #undef X
        COUNT,

        Radiation
    };
    constexpr std::string kTemperatureColorModeNames[] 
    {
    #define X(V) #V,
        TEMP_COLOR_MODE_LIST
    #undef X
    };

    struct TemperatureColor 
    {
        float tempC;
        uint32_t color; // 0xRRGGBBAA
    };
    typedef std::array<TemperatureColor, 8> TemperatureColorList;
    constexpr TemperatureColorList tempAnchors = {{
        { kAbsZero, 0x00002288 },  // black-blue
        {    0.0f, 0x00336688 },  // icy blue
        {  100.0f, 0x0066CC88 },  // cool blue
        {  300.0f, 0x80808088 },  // gray
        {  800.0f, 0x88220088 },  // dark red
        { 1500.0f, 0xFF550088 },  // orange
        { 2400.0f, 0xFFFF6688 },  // yellow-white
        { kMaxTemp, 0xFFFFFF88 }   // white-hot
    }};
    constexpr TemperatureColorList tempIRAnchors = {
        TemperatureColor{ kAbsZero, 0x000000FF }, // absolute zero - black
        TemperatureColor{ -100.0f, 0x300050FF }, // cold - deep purple
        TemperatureColor{   0.0f,  0x0040C0FF }, // cool - blue
        TemperatureColor{  100.0f, 0x00FFFFFF }, // medium - cyan
        TemperatureColor{  500.0f, 0x80FF00FF }, // warm - lime
        TemperatureColor{ 1000.0f, 0xFFFF00FF }, // hot - yellow
        TemperatureColor{ 2000.0f, 0xFF8000FF }, // very hot - red-orange
        TemperatureColor{ kMaxTemp, 0xFFFFFFFF }  // white hot
    };
    constexpr std::array<TemperatureColor, 8> blackbodyGlowAnchors = {
        TemperatureColor{    kAbsZero,  0x00000000 }, // No glow
        TemperatureColor{  525.0f,  0x2E000022 }, // Very dark red, very faint
        TemperatureColor{  700.0f,  0x80000044 }, // Dull red
        TemperatureColor{ 1000.0f,  0xFF300066 }, // Bright red
        TemperatureColor{ 1200.0f,  0xFF800088 }, // Orange
        TemperatureColor{ 1500.0f,  0xFFFF00AA }, // Yellow
        TemperatureColor{ 2000.0f,  0xFFFFFFCC }, // White hot
        TemperatureColor{ kMaxTemp,  0xA0C8FFFF }  // Blue-white, fully visible
    };
    inline uint32_t temperatureToColor(float tempC, TemperatureColorMode mode)
    {
        // Clamp to valid range
        tempC = std::clamp(tempC, tempAnchors.front().tempC, tempAnchors.back().tempC);
        const TemperatureColorList* p_anchors;
        switch (mode)
        {
        case TemperatureColorMode::Infrared:
            p_anchors = &tempIRAnchors;
            break;

        case TemperatureColorMode::Radiation:
            p_anchors = &blackbodyGlowAnchors;
            break;

        case Util::TemperatureColorMode::Normal:
        default:
            p_anchors = &tempAnchors;
            break;
        }
        const TemperatureColorList& anchors = *p_anchors;

        // Find the interval [i, i+1] where tempC lies
        for (size_t i = 0; i < anchors.size() - 1; ++i)
        {
            const auto& low = anchors[i];
            const auto& high = anchors[i + 1];

            if (tempC >= low.tempC && tempC <= high.tempC)
            {
                float t = (tempC - low.tempC) / (high.tempC - low.tempC);
                return lerpColor(low.color, high.color, t);
            }
        }

        // Fallback (should not be reached)
        return anchors.back().color;
    }

    template <size_t N>
    inline uint32_t lerpGradient(const std::array<uint32_t, N>& colors, float t)
    {
        static_assert(N >= 2, "Gradient must have at least two colors.");

        // Clamp t
        if (t <= 0.0f) return colors[0];
        if (t >= 1.0f) return colors[N - 1];

        // Scale t to color intervals
        float scaled = t * (N - 1);
        size_t index = static_cast<size_t>(scaled);
        float localT = scaled - index;

        // Edge case clamp
        if (index >= N - 1) return colors[N - 1];

        // Extract color components
        uint32_t c1 = colors[index];
        uint32_t c2 = colors[index + 1];

        uint8_t r1 = (c1 >> 24) & 0xFF, g1 = (c1 >> 16) & 0xFF, b1 = (c1 >> 8) & 0xFF, a1 = c1 & 0xFF;
        uint8_t r2 = (c2 >> 24) & 0xFF, g2 = (c2 >> 16) & 0xFF, b2 = (c2 >> 8) & 0xFF, a2 = c2 & 0xFF;

        // Interpolate
        uint8_t r = static_cast<uint8_t>(r1 + (r2 - r1) * localT);
        uint8_t g = static_cast<uint8_t>(g1 + (g2 - g1) * localT);
        uint8_t b = static_cast<uint8_t>(b1 + (b2 - b1) * localT);
        uint8_t a = static_cast<uint8_t>(a1 + (a2 - a1) * localT);

        return (r << 24) | (g << 16) | (b << 8) | a;
    }
    inline void bresenhamLine(int x0, int y0, int x1, int y1, std::function<void(int, int)> plot)
    {
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);

        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;

        int err = dx - dy;

        while (true)
        {
            plot(x0, y0); // Draw pixel

            if (x0 == x1 && y0 == y1) break;

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    constexpr float PI = 3.14159f;

}