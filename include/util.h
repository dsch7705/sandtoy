#include <cstdint>
#include <cmath>
#include <functional>


namespace Util
{
    uint32_t blendRGBA(uint32_t a, uint32_t b);

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