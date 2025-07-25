#pragma once

#include <SDL3/SDL_rect.h>
#include <vector>


// Forward Declarations //
struct SDL_Renderer;
struct SDL_Texture;
//////////////////////////
#define PARTICLE_TYPE_LIST \
    X(Air)                 \
    X(Sand)                \
    X(Gravel)

enum class ParticleType
{
#define X(NAME) NAME,
    PARTICLE_TYPE_LIST
#undef X
    COUNT
};
static const char* ParticleTypeNames[] = 
{
#define X(NAME) #NAME,
    PARTICLE_TYPE_LIST
#undef X
};

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