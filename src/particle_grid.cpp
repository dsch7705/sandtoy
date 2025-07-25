#include "particle_grid.h"

#include <SDL3/SDL.h>
#include <cassert>
#include <iostream>


ParticleGrid::ParticleGrid(const int w, const int h)
{
    assert(w > 0 && "w must be greater than 0");
    assert(h > 0 && "h must be greater than 0");

    for (int y = 0; y < h; ++y)
    {
        m_particles.emplace_back(w);
    }
}

int ParticleGrid::width() const
{
    return m_particles[0].size();
}
int ParticleGrid::height() const
{
    return m_particles.size();
}
ParticleGrid::Cell *ParticleGrid::getCell(int x, int y)
{
    if (y >= m_particles.size() || x >= m_particles[0].size())
    {
        return nullptr;
    }

    return &m_particles[y][x];
}

void ParticleGrid::draw(SDL_Renderer* pRenderer)
{
    int targetWidth, targetHeight;
    SDL_GetCurrentRenderOutputSize(pRenderer, &targetWidth, &targetHeight);

    float cellScaleX = targetWidth / (float)width();
    float cellScaleY = targetHeight / (float)height();

    for (int y = 0; y < height(); ++y)
    {
        for (int x = 0; x < width(); ++x)
        {
            SDL_FRect cellRect { .x = x * cellScaleX, .y = y * cellScaleY,
                                 .w = cellScaleX, .h = cellScaleY };
            SDL_Color cellColor;
            switch (getCell(x, y)->particleType)
            {
            default:
            case ParticleType::Air:
                cellColor = { .r = 0, .g = 0, .b = 0, .a = 0 };
                break;
            
            case ParticleType::Sand:
                cellColor = { .r = 246, .g = 215, .b = 176, .a = 255 };
                break;

            }

            SDL_SetRenderDrawColor(pRenderer, cellColor.r, cellColor.g, cellColor.b, cellColor.a);
            SDL_RenderFillRect(pRenderer, &cellRect);
        }
    }

    if (bDrawGrid)
    {
        SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 128);
        drawGrid(pRenderer, cellScaleX, cellScaleY);
    }
}
void ParticleGrid::update()
{
    update_b2t();
}

void ParticleGrid::drawGrid(SDL_Renderer *pRenderer, int scaleX, int scaleY)
{
    for (int x = 0; x <= width(); ++x)
    {
        SDL_RenderLine(pRenderer, x * scaleX, 0, x * scaleX, height() * scaleY);
    }
    for (int y = 0; y <= height(); ++y)
    {
        SDL_RenderLine(pRenderer, 0, y * scaleY, width() * scaleX, y * scaleY);
    }
}

void ParticleGrid::updateCell(int x, int y)
{
    Cell *cell = getCell(x, y);
    if (cell == nullptr)
    {
        return;
    }

    Cell *cellNext = nullptr;
    switch (cell->particleType)
    {
    case ParticleType::Sand:
        // Find suitable next cell
        if ((cellNext = getCell(x, y + 1)) == nullptr || cellNext->particleType == ParticleType::Sand)
        {
            int dir = std::rand() % 2 ? 1 : -1;
            if ((cellNext = getCell(x + dir, y + 1)) == nullptr || cellNext->particleType == ParticleType::Sand)
            {
                if ((cellNext = getCell(x - dir, y + 1)) == nullptr || cellNext->particleType == ParticleType::Sand)
                {
                    cellNext = nullptr;
                }
            }
        }
    {
        if (cellNext == nullptr)
        {
            break;
        }

        cell->particleType = ParticleType::Air;
        cellNext->particleType = ParticleType::Sand;
        break;
    }

    default:
        break;

    }
}
void ParticleGrid::update_b2t()
{
    for (int y = height(); y >= 0; --y)
    {
        for (int x = width(); x >= 0; --x)
        {
            updateCell(x, y);
        }
    }
}
void ParticleGrid::update_t2b()
{
    for (int y = 0; y < height(); --y)
    {
        for (int x = 0; x < width(); ++x)
        {
            updateCell(x, y);
        }
    }
}