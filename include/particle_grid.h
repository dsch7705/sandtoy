#pragma once

#include <SDL3/SDL_rect.h>
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
    Cell(ParticleGrid* particleGrid, int _x, int _y, ParticleType type = ParticleType::Air);

    const int x, y;
    char colorVariation;
    
    void setParticleType(ParticleType type);
    ParticleType particleType() const;

    void setSelected(bool selected);
    bool selected() const;

    void markForRedraw();

private:
    ParticleGrid* m_particleGrid;
    ParticleType m_particleType;

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

inline ParticleUpdate particleUpdateFunc_Standard(ParticleGrid* particleGrid, int x, int y)
{
    Cell* cell = particleGrid->getCell(x, y);
    if (cell == nullptr)
    {
        return { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
    }

    // Down
    Cell* cellNext = particleGrid->getCell(x, y + 1);
    if (cellNext)
    {
        int rand = std::rand();
        switch (cellNext->particleType())
        {
        case ParticleType::Air:
            return { .nextCell = cellNext, .mode = ParticleUpdate::Move };
            break;

        case ParticleType::Water:
            if (rand % 3 == 0)   // block downward movement, continue on
                break;

            return { .nextCell = cellNext, .mode = ParticleUpdate::Swap };
            break;

        default:
            break;

        }
    }
        
    // Left/right diag
    int dir = x % 2 ? 1 : -1;
    cellNext = particleGrid->getCell(x + dir, y + 1);
    if (cellNext)
    {
        int rand = std::rand();
        switch (cellNext->particleType())
        {
        case ParticleType::Air:
            return { .nextCell = cellNext, .mode = ParticleUpdate::Move };
            break;

        case ParticleType::Water:
            if (rand % 3 == 0)   // block downward movement, continue on
                break;

            return { .nextCell = cellNext, .mode = ParticleUpdate::Swap };
            break;

        default:
            break;

        }
    }

    // Left/right diag
    cellNext = particleGrid->getCell(x - dir, y + 1);
    if (cellNext)
    {
        int rand = std::rand();
        switch (cellNext->particleType())
        {
        case ParticleType::Air:
            return { .nextCell = cellNext, .mode = ParticleUpdate::Move };
            break;

        case ParticleType::Water:
            if (rand % 3 == 0)   // block downward movement, continue on
                break;

            return { .nextCell = cellNext, .mode = ParticleUpdate::Swap };
            break;

        default:
            break;

        }
    }

    return { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
}
inline ParticleUpdate particleUpdateFunc_Solid(ParticleGrid* particleGrid, int x, int y)
{
    return { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
}
inline ParticleUpdate particleUpdateFunc_Fluid(ParticleGrid* particleGrid, int x, int y)
{
    Cell* cell = particleGrid->getCell(x, y);
    if (cell == nullptr)
    {
        return { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
    }

    // Down
    Cell* cellNext = particleGrid->getCell(x, y + 1);
    if (cellNext && cellNext->particleType() == ParticleType::Air)
        return { .nextCell = cellNext, .mode = ParticleUpdate::Move };

    int dir = std::rand() % 2 ? 1 : -1;
    // Diag
    cellNext = particleGrid->getCell(x + dir, y + 1);
    if (cellNext && cellNext->particleType() == ParticleType::Air)
        return { .nextCell = cellNext, .mode = ParticleUpdate::Move };

    cellNext = particleGrid->getCell(x - dir, y + 1);
    if (cellNext && cellNext->particleType() == ParticleType::Air)
        return { .nextCell = cellNext, .mode = ParticleUpdate::Move };

    // Horizontal
    cellNext = particleGrid->getCell(x + dir, y);
    if (cellNext && cellNext->particleType() == ParticleType::Air)
        return { .nextCell = cellNext, .mode = ParticleUpdate::Move };

    cellNext = particleGrid->getCell(x - dir, y);
    if (cellNext && cellNext->particleType() == ParticleType::Air)
        return { .nextCell = cellNext, .mode = ParticleUpdate::Move };

    return { .nextCell = nullptr, .mode = ParticleUpdate::NOOP };
}
//////////////////