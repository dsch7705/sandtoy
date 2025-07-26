#pragma once

#include <SDL3/SDL_rect.h>
#include <vector>

#include "particles.h"


// Forward Declarations //
struct SDL_Renderer;
struct SDL_Texture;
//////////////////////////
struct ParticleGrid
{
    ParticleGrid(int w, int h, SDL_Renderer* renderer);
    ~ParticleGrid();

    struct Cell
    {
        Cell(int _x, int _y, ParticleType type = ParticleType::Air);

        const int x, y;
        char colorVariation;
        
    private:
        ParticleType m_particleType;
        
        friend class ParticleGrid;

    }; 
    
    int width() const;
    int height() const;

    Cell* getCell(int x, int y);
    void setCellParticleType(int x, int y, ParticleType type);
    void setCellParticleType(Cell* cell, ParticleType type);

    bool bDrawGrid { false };
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
};