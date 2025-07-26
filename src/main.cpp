#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <ctime>

#include "particle_grid.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"


// Constants //
constexpr int kCellScale { 6 };
constexpr int kGridWidth { 256 };
constexpr int kGridHeight { 128 };

constexpr int kScreenWidth { kGridWidth * kCellScale };
constexpr int kScreenHeight { kGridHeight * kCellScale };
///////////////

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    std::srand(std::time(0));
    
    SDL_Window* window = SDL_CreateWindow("SandToy", kScreenWidth, kScreenHeight, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // ImGui init //
    ImGui::CreateContext();
    ImGuiIO& guiIO = ImGui::GetIO();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    ////////////////

    ParticleGrid grid(kGridWidth, kGridHeight, renderer);

    struct Brush
    {
        int x, y;
        bool down { false };
        ParticleType particleType { ParticleType::Sand };
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
            ImGui_ImplSDL3_ProcessEvent(&e);
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

                case SDLK_R:
                    grid.clear();
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
                if (guiIO.WantCaptureMouse)
                {
                    break;
                }
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

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        //ImGui::SetNextWindowSize(ImVec2(100, 200), ImGuiCond_Once);
        ImGui::Begin("Sandbox");

        if (ImGui::BeginCombo("Particle Type", ParticleTypeNames[static_cast<int>(brush.particleType)]))
        {
            for (int i = 0; i < static_cast<int>(ParticleType::COUNT); ++i)
            {
                if (ImGui::Selectable(ParticleTypeNames[i]))
                {
                    brush.particleType = static_cast<ParticleType>(i);
                }
            }
            ImGui::EndCombo();
        }


        ImGui::End();

        // Edit Sandbox //
        if (brush.down)
        {
            grid.setCellParticleType(brush.x, brush.y, brush.particleType);
        }
        //////////////////
        // Draw //
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        grid.update();
        grid.draw();

        // ImGui
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        //////////

        SDL_RenderPresent(renderer);

        endTime = SDL_GetPerformanceCounter();
        elapsed = (float)(endTime - startTime) / SDL_GetPerformanceFrequency();
        fps = 1.f / elapsed;

        std::cout << "FPS: " << fps << std::endl;
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_Quit();
    return 0;
}