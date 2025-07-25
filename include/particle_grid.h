#pragma once

#include <vector>

// Forward Declarations //
struct SDL_Renderer;
//////////////////////////
enum class ParticleType
{
    Air,
    Sand,
};
struct ParticleGrid
{
    ParticleGrid(int w, int h);

    struct Cell
    {
        Cell() : particleType(ParticleType::Air) {}
        Cell(ParticleType type) : particleType(type) {}

        ParticleType particleType;
    }; 
    
    int width() const;
    int height() const;
    Cell *getCell(int x, int y);

    bool bDrawGrid { false };
    void draw(SDL_Renderer *pRenderer);

    void update();

private:
    std::vector<std::vector<Cell>> m_particles;

    void drawGrid(SDL_Renderer *pRenderer, int scaleX, int scaleY);

    void updateCell(int x, int y);
    void update_b2t();
    void update_t2b();
};