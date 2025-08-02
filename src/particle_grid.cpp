#include "particle_grid.h"
#include "util.h"

#include <SDL3/SDL.h>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <random>


static constexpr CellState kDefaultCellState { .temperature = 0, .temperatureDelta = 0};

Cell::Cell(ParticleGrid* particleGrid, int _x, int _y, ParticleState particleState) 
    : x(_x), y(_y)
    , m_particleState(particleState)
    , m_needsRedraw(false)
    , m_isBrushSelected(false)
    , m_cellState(kDefaultCellState)
{
    if (particleGrid == nullptr)
    {
        throw std::runtime_error("particleGrid must not be null");
    }
    m_particleGrid = particleGrid;
    colorVariation = std::rand() % 5;
}
void Cell::setParticleState(ParticleState state)
{
    if (state == m_particleState)
    {
        return;
    }
    if (state.type != m_particleState.type)
    {
        markForRedraw();
    }
    m_particleState = state;
}
ParticleState Cell::particleState() const
{
    return m_particleState;
}
void Cell::setCellState(CellState state)
{
    if (state == m_cellState)
    {
        return;
    }
    if (state.temperature != m_cellState.temperature)
    {
        markForRedraw();
    }
    m_cellState = state;
}
CellState Cell::cellState() const
{
    return m_cellState;
}

void Cell::setBrushSelected(bool selected)
{
    if (m_isBrushSelected != selected)
    {
        m_isBrushSelected = selected;
        markForRedraw();
    }
}
bool Cell::isBrushSelected() const
{
    return m_isBrushSelected;
}
void Cell::setBrushOutline(bool selected)
{
    if (m_isBrushOutline != selected)
    {
        m_isBrushOutline = selected;
        markForRedraw();
    }
}
bool Cell::isBrushOutline() const
{
    return m_isBrushOutline;
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
    : width(w)
    , height(h)
{
    assert(w > 0 && "w must be greater than 0");
    assert(h > 0 && "h must be greater than 0");

    m_particles.reserve(width * height);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            m_particles.emplace_back(this, x, y);
            m_coords.emplace_back(x, y);

            m_particles.back().markForRedraw();
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

Cell* ParticleGrid::getCell(int x, int y)
{
    if (y >= height || y < 0 || x >= width || x < 0)
    {
        return nullptr;
    }

    return &m_particles[y * width + x];
}

void ParticleGrid::draw()
{
    void* pixels;
    int pitch;
    SDL_LockTexture(m_streamingTexture, nullptr, &pixels, &pitch);
    if (pitch / sizeof(Uint32) != width)
    {
        std::cerr << "Streaming texture dimensions don't match those of the particle grid" << std::endl;
        return;
    }
    Uint32* pixelBuffer = static_cast<Uint32*>(pixels);

    for (Cell* cell : m_redrawCells)
    {
        Uint32 cellColor;
        switch (cell->particleState().type)
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

        case ParticleType::Water:
        {
            Uint32 choices[] = { 0x4DA6FF66, 0x4CA4F966, 0x4BA2F566, 0x4CA3FB66, 0x4EA7FD66 };
            cellColor = choices[cell->colorVariation];
            break;
        }

        case ParticleType::Steam:
        {
            Uint32 choices[] = { 0xFFFFFF22, 0xF5F5F522, 0xEEEEEE22, 0xE0E0E022, 0xDCDCDC22 };
            cellColor = choices[cell->colorVariation];
            break;
        }

        }

        // Add brush overlay
        if (m_showBrushHighlight && cell->isBrushSelected())
        {
            cellColor = Util::blendRGBA(cellColor, 0xFFFFFF22);
        }
        //if (cell->isBrushOutline())
        //{
        //    cellColor = Util::blendRGBA(cellColor, 0x00FF0088);
        //}
        // Ambient temp colors
        cellColor = Util::blendRGBA(cellColor, Util::lerpColor(0x0000FF99, 0xFF000099, cell->cellState().temperature / 100.f));

        pixelBuffer[cell->y * (pitch / sizeof(Uint32)) + cell->x] = cellColor;
        cell->m_needsRedraw = false;
    }
    m_redrawCells.clear();
    
    SDL_UnlockTexture(m_streamingTexture);
    SDL_RenderTexture(m_renderer, m_streamingTexture, nullptr, &m_rendererRect);
}
void ParticleGrid::update()
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    
    std::shuffle(m_coords.begin(), m_coords.end(), g);
    for (const std::pair<int, int>& coord : m_coords)
    {
        updateCell(coord.first, coord.second);
    }
    
    std::vector<float> accumulatedDelta(m_particles.size(), 0.f);

    // Phase 1: calculate deltas from neighbors
    for (const auto& coord : m_coords)
    {
        Cell* cell = getCell(coord.first, coord.second);
        if (!cell) continue;

        CellState cellState = cell->cellState();

        std::vector<Cell*> neighbors;
        auto tryPush = [&](int x, int y)
        {
            Cell* c = getCell(x, y);
            if (c)
            {
                neighbors.push_back(c);
            }
        };
        tryPush(coord.first + 1, coord.second);
        tryPush(coord.first + 1, coord.second + 1);
        tryPush(coord.first, coord.second + 1);
        tryPush(coord.first - 1, coord.second + 1);
        tryPush(coord.first - 1, coord.second);
        tryPush(coord.first - 1, coord.second - 1);
        tryPush(coord.first, coord.second - 1);
        tryPush(coord.first + 1, coord.second - 1);

        for (Cell* neighbor : neighbors)
        {
            CellState neighborState = neighbor->cellState();

            float delta = (neighborState.temperature - cellState.temperature) * 0.01f;

            accumulatedDelta[cell->y * width + cell->x] += delta;
            accumulatedDelta[neighbor->y * width + neighbor->x] -= delta;
        }
    }

    // Phase 2: apply accumulated deltas
    for (int i = 0; i < (int)m_particles.size(); ++i)
    {
        CellState state = m_particles[i].cellState();
        state.temperatureDelta += accumulatedDelta[i];
        m_particles[i].setCellState(state);
    }

    // Phase 3: update temperatures and reset deltas
    for (Cell& cell : m_particles)
    {
        CellState state = cell.cellState();
        state.temperature += state.temperatureDelta;
        state.temperatureDelta = 0.f;
        cell.setCellState(state);
    }

    //update_b2t();
}
void ParticleGrid::clear(ParticleType type)
{
    for (Cell& cell : m_particles)
    {
        cell.setParticleState(defaultParticleState(type));
    }
}

void ParticleGrid::updateCell(int x, int y)
{
    Cell* cell = getCell(x, y);
    if (cell == nullptr)
    {
        return;
    }

    // Positioning
    ParticleUpdate update = { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
    switch (ParticlePhases[(int)(cell->particleState().type)])
    {
    case ParticlePhase::Solid:
        update = particleUpdateFunc_Solid(this, x, y);
        break;
    
    case ParticlePhase::Liquid:
        update = particleUpdateFunc_Liquid(this, x, y);
        break;
    
    case ParticlePhase::Gas:
        update = particleUpdateFunc_Gas(this, x, y);
        break;

    case ParticlePhase::Static:
        update = particleUpdateFunc_Static(this, x, y);
        break;

    default:
        break;

    }

    switch (update.mode)
    {
    case ParticleUpdate::Move:
        update.nextCell->setParticleState(cell->particleState());
        cell->setParticleState({ .type = ParticleType::Air, .temperature = 0 });
        break;

    case ParticleUpdate::Swap:
    {
        ParticleState tmp = cell->particleState();
        cell->setParticleState(update.nextCell->particleState());
        update.nextCell->setParticleState(tmp);
        break;
    }

    case ParticleUpdate::NOOP:
    default:
        break;

    }
}
void ParticleGrid::update_b2t()
{
    for (int y = height; y >= 0; --y)
    {
        int scanDir = (y % 2) ? 1 : -1;
        int startX = (scanDir == 1) ? 0 : width;
        int endX = (startX == 0) ? width : -1;

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
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Cell* cell = getCell(x, y);
            if (cell != nullptr)
            {
                updateCell(cell->x, cell->y);
            }
        }
    }
}