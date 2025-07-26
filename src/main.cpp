#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <ctime>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "particle_grid.h"
#include "brush.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"


// Constants //
constexpr int kCellScale { 3 };
constexpr int kGridWidth { 512 };
constexpr int kGridHeight { 256 };

constexpr int kScreenWidth { kGridWidth * kCellScale };
constexpr int kScreenHeight { kGridHeight * kCellScale };
///////////////

static SDL_Window* window;
static SDL_Renderer* renderer;

static Uint64 startTime, endTime;
static float deltaTime, fps;

static ParticleGrid* grid;
static Brush* brush;
static ImGuiIO* guiIO;

static bool quit { false };

static void mainloop()
{
    startTime = SDL_GetPerformanceCounter();

    // Handle Events //
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        ImGui_ImplSDL3_ProcessEvent(&e);
        brush->handleEvent(&e, guiIO->WantCaptureMouse);

        switch (e.type)
        {
        
        case SDL_EVENT_QUIT:
            quit = true;
            break;

        case SDL_EVENT_KEY_DOWN:
            switch (e.key.key)
            {
            case SDLK_G:
                grid->toggleGridLines();
                break;

            case SDLK_H:
                brush->toggleHighlight();
                break;

            case SDLK_R:
                grid->clear();
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
    brush->stroke();
    grid->update();
    ////////////

    // Edit Sandbox //
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Sandbox", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(100.f);

    if (ImGui::BeginCombo("Particle Type", ParticleTypeNames[static_cast<int>(brush->particleType())]))
    {
        for (int i = 0; i < static_cast<int>(ParticleType::COUNT); ++i)
        {
            if (ImGui::Selectable(ParticleTypeNames[i]))
            {
                brush->setParticleType(static_cast<ParticleType>(i));
                //brush.particleType = static_cast<ParticleType>(i);
            }
        }
        ImGui::EndCombo();
    }

    float guiBrushRadius = brush->radius();
    if (ImGui::SliderFloat("Brush radius", &guiBrushRadius, Brush::kMinRadius, Brush::kMaxRadius, "%.1f"))
    {
        brush->setRadius(guiBrushRadius);
    }

    bool guiShowGridLines = grid->gridLines();
    if (ImGui::Checkbox("Show grid lines", &guiShowGridLines))
    {
        grid->toggleGridLines();
    }

    bool guiShowBrushHighlight = brush->highlight();
    if (ImGui::Checkbox("Show brush highlight", &guiShowBrushHighlight))
    {
        brush->toggleHighlight();
    }

    ImGui::Text("FPS: %f", fps);

    ImGui::PopItemWidth();
    ImGui::End();

    //////////////////
    // Draw //
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    grid->draw();

    endTime = SDL_GetPerformanceCounter();
    deltaTime = static_cast<float>(endTime - startTime) / SDL_GetPerformanceFrequency();
#ifdef EMSCRIPTEN
    deltaTime = std::max(deltaTime, 0.001f);
#endif
    fps = 1.f / deltaTime;

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    std::srand(std::time(0));
    
    window = SDL_CreateWindow("SandToy", kScreenWidth, kScreenHeight, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // ImGui init //
    ImGui::CreateContext();
    guiIO = &ImGui::GetIO();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    //ImFont* roboto_med = guiIO->Fonts->AddFontFromFileTTF(IMGUI_FONTS_DIR"/Roboto-Medium.ttf");
    //roboto_med->Scale = 0.75f;
    //guiIO->Fonts->Build();
    ////////////////

    grid = new ParticleGrid(kGridWidth, kGridHeight, renderer);
    brush = new Brush(5.f, ParticleType::Sand);
    brush->setCanvas(grid);

#ifdef EMSCRIPTEN
    emscripten_set_main_loop(mainloop, 0, 1);
#else
    while (!quit) { mainloop(); }
#endif
    
    delete brush;
    delete grid;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_Quit();
    return 0;
}