#pragma once

#include <SDL3/SDL_rect.h>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "particles.h"


// Forward Declarations //
struct SDL_Renderer;
struct SDL_Texture;

struct ParticleGrid;
class Brush;
//////////////////////////
struct Cell
{
    Cell(ParticleGrid* particleGrid, int _x, int _y, ParticleState particleState = defaultParticleState(ParticleType::Air));

    const int x, y;
    char colorVariation;
    
    void setParticleState(ParticleState state);
    ParticleState particleState() const;

    void setSelected(bool selected);
    bool selected() const;

    void markForRedraw();

private:
    ParticleGrid* m_particleGrid;
    ParticleState m_particleState;
    //ParticleType m_particleType;

    bool m_needsRedraw;
    bool m_isSelected;
    
    friend class ParticleGrid;

};
struct ParticleGrid
{
    ParticleGrid(int w, int h, SDL_Renderer* renderer);
    ~ParticleGrid(); 
    
    int width() const;
    int height() const;

    Cell* getCell(int x, int y);
    
    void draw();
    void update();
    void clear(ParticleType type = ParticleType::Air);

private:
    std::vector<std::vector<Cell>> m_particles;
    std::vector<Cell*> m_redrawCells;

    SDL_Texture* m_streamingTexture;
    SDL_Renderer* m_renderer;
    SDL_FRect m_rendererRect;

    bool m_showBrushHighlight { true };

    void updateCell(int x, int y);
    void update_b2t();
    void update_t2b();

    friend class Brush;
    friend class Cell;

};

// Update Funcs //
struct ParticleUpdate
{
    Cell* nextCell;
    enum ParticleUpdateMode
    {
        Move,
        Swap,
        NOOP
    } mode;
};
constexpr ParticleUpdate doNothing { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };

inline ParticleUpdate particleUpdateFunc_Solid(ParticleGrid* particleGrid, int x, int y)
{
    Cell* cell = particleGrid->getCell(x, y);
    if (cell == nullptr)
    {
        return doNothing;
    }

    Cell* cellNext = nullptr;

    #define TRY_UPDATE() \
    do { \
        if (cellNext) \
        { \
            int rand = std::rand(); \
            switch (cellNext->particleState().type) \
            { \
            case ParticleType::Air: \
                if (rand % 15 == 0) { break; } \
                return { .nextCell = cellNext, .mode = ParticleUpdate::Move }; \
                break; \
            case ParticleType::Water: \
                if (rand % 3 == 0) { break; } \
                return { .nextCell = cellNext, .mode = ParticleUpdate::Swap }; \
                break; \
            default: \
                break; \
            } \
        } \
    } while (0)

    // Down
    cellNext = particleGrid->getCell(x, y + 1);
    TRY_UPDATE();
        
    // Left/right diag
    int dir = x % 2 ? 1 : -1;
    cellNext = particleGrid->getCell(x + dir, y + 1);
    TRY_UPDATE();

    // Left/right diag
    cellNext = particleGrid->getCell(x - dir, y + 1);
    TRY_UPDATE();

    #undef TRY_UPDATE

    return doNothing;
}
inline ParticleUpdate particleUpdateFunc_Liquid(ParticleGrid* particleGrid, int x, int y)
{
    Cell* cell = particleGrid->getCell(x, y);
    if (cell == nullptr)
    {
        return doNothing;
    }

    Cell* cellNext = nullptr;

    #define TRY_UPDATE() \
    do { \
        if (cellNext) \
        { \
            int rand = std::rand(); \
            switch (cellNext->particleState().type) \
            { \
            case ParticleType::Air: \
                if (rand % 30 == 0) { break; } \
                return { .nextCell = cellNext, .mode = ParticleUpdate::Move }; \
                break; \
            default: \
                break; \
            } \
        } \
    } while (0)

    // Down
    cellNext = particleGrid->getCell(x, y + 1);
    TRY_UPDATE();
        
    // diag
    int dir = std::rand() % 2 ? 1 : -1;
    cellNext = particleGrid->getCell(x + dir, y + 1);
    TRY_UPDATE();

    cellNext = particleGrid->getCell(x - dir, y + 1);
    TRY_UPDATE();

    // horizontal
    cellNext = particleGrid->getCell(x + dir, y);
    TRY_UPDATE();

    cellNext = particleGrid->getCell(x - dir, y);
    TRY_UPDATE();

    #undef TRY_UPDATE

    return doNothing;
}
inline ParticleUpdate particleUpdateFunc_Gas(ParticleGrid* particleGrid, int x, int y)
{
    Cell* cell = particleGrid->getCell(x, y);
    if (cell == nullptr)
    {
        return { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
    }

    Cell* cellNext = nullptr;

    int rand = std::rand();
    if (rand % 50 == 0)
    {
        ParticleState nextState = cell->particleState();
        if (--nextState.life == 0)
        {
            cell->setParticleState(defaultParticleState(ParticleType::Water));
            return doNothing;
        }
        cell->setParticleState(nextState);
    }

    switch (rand % 7)
    {
    case 0:
        cellNext = particleGrid->getCell(x, y - 1);
        break;
    
    case 1:
        cellNext = particleGrid->getCell(x - 1, y - 1);
        break;

    case 2:
        cellNext = particleGrid->getCell(x + 1, y - 1);
        break;

    case 3:
        cellNext = particleGrid->getCell(x - 1, y);
        break;

    case 4:
        cellNext = particleGrid->getCell(x + 1, y);
        break;

    case 5:
        cellNext = particleGrid->getCell(x - 1, y + 1);
        break;

    case 6:
        cellNext = particleGrid->getCell(x + 1, y + 1);
        break;

    default:
        cellNext = nullptr;
        break;

    }

    if (!cellNext)
    {
        return { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
    }

    if (cellNext->particleState().type == ParticleType::Air)
    {
        return { .nextCell = cellNext, .mode = ParticleUpdate::Move };
    }

    return { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
}
inline ParticleUpdate particleUpdateFunc_Static(ParticleGrid* particleGrid, int x, int y)
{
    return doNothing;
}
//////////////////