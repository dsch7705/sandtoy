#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

#include "particle_grid.h"


// Constants //
constexpr int kCellScale { 8 };
constexpr int kGridWidth { 128 };
constexpr int kGridHeight { 64 };

constexpr int kScreenWidth { kGridWidth * kCellScale };
constexpr int kScreenHeight { kGridHeight * kCellScale };
///////////////

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window* pWindow = SDL_CreateWindow("SandToy", kScreenWidth, kScreenHeight, 0);
    SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, nullptr);
    SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);

    ParticleGrid grid(kGridWidth, kGridHeight, pRenderer);

    struct Brush
    {
        int x, y;
        bool down { false };
    } brush;

    Uint64 startTime, endTime;
    float elapsed, fps;

    bool bQuit { false };
    while (!bQuit)
    {
        startTime = SDL_GetPerformanceCounter();

        // Handle Events //
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_EVENT_QUIT:
                bQuit = true;
                break;

            case SDL_EVENT_KEY_DOWN:
                switch (e.key.key)
                {
                case SDLK_G:
                    grid.bDrawGrid = !grid.bDrawGrid;
                    break;

                default:
                    break;

                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                switch(e.button.button)
                {
                case 1:
                    brush.down = false;
                    break;
                
                default:
                    break;

                }
                break;
                
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                switch (e.button.button)
                {
                case 1:
                    brush.down = true;
                    break;

                default:
                    break;

                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                brush.x = ((float)e.motion.x / kScreenWidth) * kGridWidth;
                brush.y = ((float)e.motion.y / kScreenHeight) * kGridHeight;

            default:
                break;

            }
        }
        ///////////////////
        // Edit Sandbox //
        if (brush.down)
        {
            ParticleGrid::Cell* cell = grid.getCell(brush.x, brush.y);
            if (cell)
            {
                cell->setParticleType(ParticleType::Sand);
            }
        }
        //////////////////
        // Draw //
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        grid.update();
        grid.draw();

        SDL_RenderPresent(pRenderer);
        //////////

        endTime = SDL_GetPerformanceCounter();
        elapsed = (float)(endTime - startTime) / SDL_GetPerformanceFrequency();
        fps = 1.f / elapsed;

        std::cout << "FPS: " << fps << std::endl;
    }
    
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    
    SDL_Quit();
    return 0;
}