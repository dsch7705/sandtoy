#include "particle_grid.h"
#include "util.h"

#include <SDL3/SDL.h>
#include <cassert>
#include <iostream>


Cell::Cell(ParticleGrid* particleGrid, int _x, int _y, ParticleType type) 
    : x(_x), y(_y)
    , m_particleType(type)
    , m_needsRedraw(false)
    , m_isSelected(false)
{
    if (particleGrid == nullptr)
    {
        throw std::runtime_error("particleGrid must not be null");
    }
    m_particleGrid = particleGrid;
    colorVariation = std::rand() % 5;
}
void Cell::setParticleType(ParticleType type)
{
    if (type != m_particleType)
    {
        m_particleType = type;
        markForRedraw();
    }
}
ParticleType Cell::particleType() const
{
    return m_particleType;
}
void Cell::setSelected(bool selected)
{
    if (m_isSelected != selected)
    {
        m_isSelected = selected;
        markForRedraw();
    }
}
bool Cell::selected() const
{
    return m_isSelected;
}

void Cell::markForRedraw()
{
    if (!m_needsRedraw)
    {
        m_particleGrid->m_redrawCells.push_back(this);
        m_needsRedraw = true;
    }
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
            row.emplace_back(this, x, y);
            //row.back().markForRedraw();
        }

        m_particles.push_back(std::move(row));
        for (Cell& cell : m_particles.back())
        {
            cell.markForRedraw();
        }
    }

    m_renderer = renderer;
    int rW, rH;
    SDL_GetCurrentRenderOutputSize(m_renderer, &rW, &rH);
    m_rendererRect = { .x = 0, .y = 0, .w = static_cast<float>(rW), .h = static_cast<float>(rH) };

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

Cell* ParticleGrid::getCell(int x, int y)
{
    if (y >= m_particles.size() || x >= m_particles[0].size())
    {
        return nullptr;
    }

    return &m_particles[y][x];
}

void ParticleGrid::draw()
{
    void* pixels;
    int pitch;
    SDL_LockTexture(m_streamingTexture, nullptr, &pixels, &pitch);
    if (pitch / sizeof(Uint32) != width())
    {
        std::cerr << "Streaming texture dimensions don't match those of the particle grid" << std::endl;
        return;
    }
    Uint32* pixelBuffer = static_cast<Uint32*>(pixels);

    for (Cell* cell : m_redrawCells)
    {
        Uint32 cellColor;
        switch (cell->m_particleType)
        {
        default:
            cellColor = 0xFF00FFFF;
            break;

        case ParticleType::Vacuum:
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

        case ParticleType::Rainbow:
        {
            Uint32 choices[] = { 0xEF476FFF, 0xFFA600FF, 0x06D6A0FF, 0x118AB2FF, 0x9B5DE5FF };
            cellColor = choices[cell->colorVariation];
            break;
        }

        case ParticleType::Pink:
        {
            Uint32 choices[] = { 0xFFC0CBFF, 0xFFB6C1FF, 0xFF69B4FF, 0xFF1493FF, 0xDB7093FF };
            cellColor = choices[cell->colorVariation];
            break;
        }

        case ParticleType::Blue:
        {
            Uint32 choices[] = { 0x3A75C4FF, 0x4682B4FF, 0x5B9BD5FF, 0x4F83CCFF, 0x357EC7FF };
            cellColor = choices[cell->colorVariation];
            break;
        }   

        }

        // Add brush overlay
        if (m_showBrushHighlight && cell->selected())
        {
            cellColor = Util::blendRGBA(cellColor, 0xFFFFFF22);
        }

        pixelBuffer[cell->y * (pitch / sizeof(Uint32)) + cell->x] = cellColor;
        cell->m_needsRedraw = false;
    }
    m_redrawCells.clear();
    
    SDL_UnlockTexture(m_streamingTexture);
    SDL_RenderTexture(m_renderer, m_streamingTexture, nullptr, &m_rendererRect);
}
void ParticleGrid::update()
{
    update_b2t();
    //update_t2b();
}
void ParticleGrid::clear(ParticleType type)
{
    for (std::vector<Cell>& row : m_particles)
    {
        for (Cell& cell : row)
        {
            cell.setParticleType(type);
        }
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
    case ParticleType::Rainbow:
    case ParticleType::Pink:
    case ParticleType::Blue:

    case ParticleType::Stone:
    case ParticleType::Gravel:
    case ParticleType::Dirt:
    case ParticleType::Sand:
        // Find suitable next cell
        if ((cellNext = getCell(x, y + 1)) != nullptr && cellNext->m_particleType != ParticleType::Vacuum)
        {
            int dir = std::rand() % 2 ? 1 : -1;
            if ((cellNext = getCell(x + dir, y + 1)) != nullptr && cellNext->m_particleType != ParticleType::Vacuum)
            {
                if ((cellNext = getCell(x - dir, y + 1)) != nullptr && cellNext->m_particleType != ParticleType::Vacuum)
                {
                    cellNext = nullptr;
                }
            }
        }
    
        if (cellNext != nullptr)
        {
            cellNext->setParticleType(cell->particleType());
            cell->setParticleType(ParticleType::Vacuum);
        }
        break;
    
    case ParticleType::Vacuum:
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
        int scanDir = (y % 2) ? 1 : -1;
        int startX = (scanDir == 1) ? 0 : width();
        int endX = (startX == 0) ? width() : -1;

        int x = startX;
        while (x != endX)
        {
            Cell* cell = getCell(x, y);
            if (cell != nullptr)
            {
                updateCell(cell->x, cell->y);
            }

            x += scanDir;
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