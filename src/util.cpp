#include "util.h"


uint32_t Util::blendRGBA(uint32_t a, uint32_t b)
{
    uint8_t aR, aG, aB, aA,
          bR, bG, bB, bA,
          cR, cG, cB, cA;

    aR = (a >> 24) & 0xFF;
    aG = (a >> 16) & 0xFF;
    aB = (a >> 8)  & 0xFF;
    aA = (a >> 0)  & 0xFF;

    bR = (b >> 24) & 0xFF;
    bG = (b >> 16) & 0xFF;
    bB = (b >> 8)  & 0xFF;
    bA = (b >> 0)  & 0xFF;

    float alpha = bA / 255.f;

    cR = static_cast<uint8_t>(bR * alpha + aR * (1.f - alpha));
    cG = static_cast<uint8_t>(bG * alpha + aG * (1.f - alpha));
    cB = static_cast<uint8_t>(bB * alpha + aB * (1.f - alpha));
    cA = 255;

    return (cR << 24) | (cG << 16) | (cB << 8) | (cA << 0);
}