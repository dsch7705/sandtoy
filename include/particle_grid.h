#pragma once

#include <SDL3/SDL_rect.h>
#include <vector>


// Forward Declarations //
struct SDL_Renderer;
struct SDL_Texture;
//////////////////////////
enum class ParticleType
{
    Air,
    Sand,
};
struct ParticleGrid
{
    ParticleGrid(int w, int h, SDL_Renderer* renderer);
    ~ParticleGrid();

    struct Cell
    {
        Cell(int _x, int _y, ParticleType type = ParticleType::Air);

        void setParticleType(ParticleType type);
        ParticleType particleType() const;

        const int x, y;
        char colorVariation;
        
    private:
        ParticleType m_particleType;

    }; 
    
    int width() const;
    int height() const;

    Cell* getCell(int x, int y);

    bool bDrawGrid { false };
    void draw();

    void update();

private:
    std::vector<std::vector<Cell>> m_particles;
    SDL_Texture* m_streamingTexture;
    SDL_Renderer* m_renderer;
    SDL_FRect m_rendererRect;

    void drawGrid();

    void updateCell(int x, int y);
    void update_b2t();
    void update_t2b();
};