#include <cstdint>
#include <cmath>
#include <functional>


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

    static constexpr float PI = 3.14159f;

}