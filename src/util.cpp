#include "util.h"
#include "SDL3/SDL_pixels.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <iostream>
#include <iterator>


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
SDL_Texture* Util::scaleTexture(SDL_Renderer* renderer, SDL_Texture* in, unsigned int newW, unsigned int newH)
{
    if (in == nullptr)
    {
        std::cerr << __func__ << ": Input texture is null\n";
        return nullptr;
    }
    else if (newW == 0 || newH == 0)
    {
        std::cerr << __func__ << ": New size (" << newW << ", " << newH << ") is invalid\n";
        return nullptr;
    }

    SDL_Texture* newTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, newW, newH);
    if (!newTex)
    {
        std::cerr << __func__ << ": Failed to create new texture\n";
        return nullptr;
    }

    SDL_FRect newSize { 0.f, 0.f, (float)newW, (float)newH };

    SDL_Texture* currentTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, newTex);
    SDL_RenderTexture(renderer, in, nullptr, &newSize);
    SDL_SetRenderTarget(renderer, currentTarget);

    return newTex;
}
