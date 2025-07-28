#pragma once

#include <SDL3/SDL_rect.h>
#include <vector>

#include "particles.h"


// Forward Declarations //
struct SDL_Renderer;
struct SDL_Texture;

struct ParticleGrid;
class Brush;
//////////////////////////
struct Cell
{
    Cell(ParticleGrid* particleGrid, int _x, int _y, ParticleType type = ParticleType::Vacuum);

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
    void clear(ParticleType type = ParticleType::Vacuum);

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
inline Cell* particleUpdateFunc_Standard(ParticleGrid* particleGrid, int x, int y)
{
    Cell* cell = particleGrid->getCell(x, y);
    if (cell == nullptr)
    {
        return nullptr;
    }

    Cell* cellNext = nullptr;
    // Find suitable next cell
    if ((cellNext = particleGrid->getCell(x, y + 1)) != nullptr && cellNext->particleType() != ParticleType::Vacuum)
    {
        int dir = y % 2 ? 1 : -1;
        if ((cellNext = particleGrid->getCell(x + dir, y + 1)) != nullptr && cellNext->particleType() != ParticleType::Vacuum)
        {
            if ((cellNext = particleGrid->getCell(x - dir, y + 1)) != nullptr && cellNext->particleType() != ParticleType::Vacuum)
            {
                cellNext = nullptr;
            }
        }
    }

    return cellNext;
}
inline Cell* particleUpdateFunc_Solid(ParticleGrid* particleGrid, int x, int y)
{
    return nullptr;
}
//////////////////