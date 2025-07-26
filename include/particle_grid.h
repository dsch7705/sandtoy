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

private:
    ParticleGrid* m_particleGrid;
    ParticleType m_particleType;

    bool m_needsRedraw;
    bool m_isSelected;

    void markForRedraw();
    
    friend class ParticleGrid;

};
struct ParticleGrid
{
    ParticleGrid(int w, int h, SDL_Renderer* renderer);
    ~ParticleGrid(); 
    
    int width() const;
    int height() const;

    Cell* getCell(int x, int y);

    bool bDrawGrid { false };
    bool bHighlightSelected { true };
    void draw();

    void update();
    void clear();

private:
    std::vector<std::vector<Cell>> m_particles;
    std::vector<Cell*> m_redrawCells;

    SDL_Texture* m_streamingTexture;
    SDL_Renderer* m_renderer;
    SDL_FRect m_rendererRect;

    void drawGrid();

    void updateCell(int x, int y);
    void update_b2t();
    void update_t2b();

    friend class Brush;
    friend class Cell;

};