#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <ctime>

#include "particle_grid.h"
#include "brush.h"

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

    ImFont* roboto_med = guiIO.Fonts->AddFontFromFileTTF(IMGUI_FONTS_DIR"/Roboto-Medium.ttf");
    roboto_med->Scale = 0.75f;
    guiIO.Fonts->Build();
    ////////////////

    ParticleGrid grid(kGridWidth, kGridHeight, renderer);
    Brush brush(5.f, ParticleType::Sand);
    brush.setCanvas(&grid);


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
            brush.handleEvent(&e, guiIO.WantCaptureMouse);

            switch (e.type)
            {
            case SDL_EVENT_QUIT:
                bQuit = true;
                break;

            case SDL_EVENT_KEY_DOWN:
                switch (e.key.key)
                {
                case SDLK_G:
                    grid.toggleGridLines();
                    break;

                case SDLK_H:
                    brush.toggleHighlight();
                    break;

                case SDLK_R:
                    grid.clear();
                    break;

                default:
                    break;

                }
                break;

            default:
                break;

            }
        }
        ///////////////////

        // Update //
        brush.stroke();
        grid.update();
        ////////////

        // Edit Sandbox //
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Sandbox", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::PushItemWidth(100.f);

        if (ImGui::BeginCombo("Particle Type", ParticleTypeNames[static_cast<int>(brush.particleType())]))
        {
            for (int i = 0; i < static_cast<int>(ParticleType::COUNT); ++i)
            {
                if (ImGui::Selectable(ParticleTypeNames[i]))
                {
                    brush.setParticleType(static_cast<ParticleType>(i));
                    //brush.particleType = static_cast<ParticleType>(i);
                }
            }
            ImGui::EndCombo();
        }

        float guiBrushRadius = brush.radius();
        if (ImGui::SliderFloat("Brush radius", &guiBrushRadius, Brush::kMinRadius, Brush::kMaxRadius, "%.1f"))
        {
            brush.setRadius(guiBrushRadius);
        }

        bool guiShowGridLines = grid.gridLines();
        if (ImGui::Checkbox("Show grid lines", &guiShowGridLines))
        {
            grid.toggleGridLines();
        }

        bool guiShowBrushHighlight = brush.highlight();
        if (ImGui::Checkbox("Show brush highlight", &guiShowBrushHighlight))
        {
            brush.toggleHighlight();
        }

        ImGui::Text("FPS: %f", fps);

        ImGui::PopItemWidth();
        ImGui::End();

        //////////////////
        // Draw //
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        grid.draw();

        endTime = SDL_GetPerformanceCounter();
        elapsed = (float)(endTime - startTime) / SDL_GetPerformanceFrequency();
        fps = 1.f / elapsed;

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_Quit();
    return 0;
}