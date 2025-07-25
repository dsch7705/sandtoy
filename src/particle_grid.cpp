#include "particle_grid.h"

#include <SDL3/SDL.h>
#include <cassert>
#include <algorithm>
#include <iostream>


ParticleGrid::Cell::Cell(int _x, int _y, ParticleType type) 
    : x(_x), y(_y)
    , m_particleType(type)
{
    colorVariation = std::rand() % 5;
}

ParticleGrid::ParticleGrid(const int w, const int h, SDL_Renderer* renderer)
{
    assert(w > 0 && "w must be greater than 0");
    assert(h > 0 && "h must be greater than 0");

    for (int y = 0; y < h; ++y)
    {
        std::vector<Cell> row;
        for (int x = 0; x < w; ++x)
        {
            row.emplace_back(x, y);
        }

        m_particles.push_back(std::move(row));
    }

    m_renderer = renderer;
    int rW, rH;
    SDL_GetCurrentRenderOutputSize(m_renderer, &rW, &rH);
    m_rendererRect = { .x = 0, .y = 0, .w = (float)rW, .h = (float)rH };

    m_streamingTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    SDL_SetTextureScaleMode(m_streamingTexture, SDL_SCALEMODE_NEAREST);
}
ParticleGrid::~ParticleGrid()
{
    SDL_DestroyTexture(m_streamingTexture);
}

int ParticleGrid::width() const
{
    return m_particles[0].size();
}
int ParticleGrid::height() const
{
    return m_particles.size();
}

ParticleGrid::Cell* ParticleGrid::getCell(int x, int y)
{
    if (y >= m_particles.size() || x >= m_particles[0].size())
    {
        return nullptr;
    }

    return &m_particles[y][x];
}
void ParticleGrid::setCellParticleType(int x, int y, ParticleType type)
{
    Cell* cell = getCell(x, y);
    setCellParticleType(cell, type);
}
void ParticleGrid::setCellParticleType(Cell* cell, ParticleType type)
{
    if (cell)
    {
        cell->m_particleType = type;
        m_redrawCells.push_back(cell);
    }
}

void ParticleGrid::draw()
{
    void* pixels;
    int pitch;
    SDL_LockTexture(m_streamingTexture, nullptr, &pixels, &pitch);
    if (pitch / sizeof(Uint32) != width())
    {
        std::cout << "Streaming texture dimensions don't match those of the particle grid" << std::endl;
        return;
    }
    Uint32* pixelBuffer = static_cast<Uint32*>(pixels);

    for (const Cell* cell : m_redrawCells)
    {
        Uint32 cellColor;
        switch (cell->m_particleType)
        {
        default:
            cellColor = 0xFF00FFFF;
            break;

        case ParticleType::Air:
            cellColor = 0x00000000;
            break;
        
        case ParticleType::Stone:
        {
            Uint32 choices[] = { 0x4A4A4AFF, 0x505050FF, 0x464646FF, 0x4C4C4CFF, 0x444444FF };
            cellColor = choices[cell->colorVariation];
            break;
        }

        case ParticleType::Gravel:
        {
            //Uint32 choices[] = { 0x3A3128FF, 0x615441FF, 0x89785CFF, 0x333333FF, 0x272727FF };
            Uint32 choices[] = { 0x6A6A6AFF, 0x707070FF, 0x666666FF, 0x5E5E5EFF, 0x747474FF };
            cellColor = choices[cell->colorVariation];
            break;
        }

        case ParticleType::Dirt:
        {
            Uint32 choices[] = { 0x5A3A1EFF, 0x684425FF, 0x4E3018FF, 0x6F482BFF, 0x59391FFF };
            cellColor = choices[cell->colorVariation];
            break;
        }

        case ParticleType::Sand:
        {
            //Uint32 choices[] = { 0xF6D7B0FF, 0xF2D2A9FF, 0xECCCA2FF, 0xE7C496FF, 0xE1BF92FF };
            Uint32 choices[] = { 0xE2C290FF, 0xD6B77EFF, 0xF0D8A8FF, 0xCCAA72FF, 0xB8935EFF };
            cellColor = choices[cell->colorVariation];
            break;
        }

        }

        pixelBuffer[cell->y * (pitch / sizeof(Uint32)) + cell->x] = cellColor;
        m_redrawCells.clear();
    }

    SDL_UnlockTexture(m_streamingTexture);
    SDL_RenderTexture(m_renderer, m_streamingTexture, nullptr, &m_rendererRect);

    if (bDrawGrid)
    {
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 128);
        drawGrid();
    }
}
void ParticleGrid::update()
{
    update_b2t();
    //update_t2b();
}

void ParticleGrid::drawGrid()
{
    int scaleX, scaleY;
    SDL_GetCurrentRenderOutputSize(m_renderer, &scaleX, &scaleY);
    scaleX /= width();
    scaleY /= height();

    for (int x = 0; x <= width(); ++x)
    {
        SDL_RenderLine(m_renderer, x * scaleX, 0, x * scaleX, height() * scaleY);
    }
    for (int y = 0; y <= height(); ++y)
    {
        SDL_RenderLine(m_renderer, 0, y * scaleY, width() * scaleX, y * scaleY);
    }
}

void ParticleGrid::updateCell(int x, int y)
{
    Cell* cell = getCell(x, y);
    if (cell == nullptr)
    {
        return;
    }

    Cell* cellNext = nullptr;
    switch (cell->m_particleType)
    {
    case ParticleType::Stone:
    case ParticleType::Gravel:
    case ParticleType::Dirt:
    case ParticleType::Sand:
        // Find suitable next cell
        if ((cellNext = getCell(x, y + 1)) != nullptr && cellNext->m_particleType != ParticleType::Air)
        {
            int dir = std::rand() % 2 ? 1 : -1;
            if ((cellNext = getCell(x + dir, y + 1)) != nullptr && cellNext->m_particleType != ParticleType::Air)
            {
                if ((cellNext = getCell(x - dir, y + 1)) != nullptr && cellNext->m_particleType != ParticleType::Air)
                {
                    cellNext = nullptr;
                }
            }
        }
    
        if (cellNext != nullptr)
        {
            setCellParticleType(cellNext, cell->m_particleType);
            setCellParticleType(cell, ParticleType::Air);
        }
        break;
    
    case ParticleType::Air:
        //m_activeParticles.remove(cell);
        break;

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
            Cell* cell = getCell(x, y);
            if (cell != nullptr)
            {
                updateCell(cell->x, cell->y);
            }
        }
    }
}
void ParticleGrid::update_t2b()
{
    for (int y = 0; y < height(); ++y)
    {
        for (int x = 0; x < width(); ++x)
        {
            Cell* cell = getCell(x, y);
            if (cell != nullptr)
            {
                updateCell(cell->x, cell->y);
            }
        }
    }
}