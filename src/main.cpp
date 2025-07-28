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

constexpr int kFrameCap { 20 };
constexpr double kFrameDuration { 1. / kFrameCap };
///////////////

static SDL_Window* window;
static SDL_Renderer* renderer;

static Uint64 startTime, endTime;
static double deltaTime, fps;

static ParticleGrid* grid;
static Brush* brush;
static ImGuiIO* guiIO;

static float guiBrushRadius;
static bool guiShowBrushHighlight;
static bool guiShowControls { false };
static bool guiShowFPS { true };

static Uint64 freq = SDL_GetPerformanceFrequency();

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
            case SDLK_R:
                grid->clear();
                break;

            case SDLK_H:
                brush->toggleHighlight();
                break;

            case SDLK_C:
                if (e.key.mod & SDL_KMOD_ALT)
                {
                    guiShowControls = !guiShowControls;
                    break;
                }

            case SDLK_F:
                if (e.key.mod & SDL_KMOD_ALT)
                {
                    guiShowFPS = !guiShowFPS;
                    break;
                }

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

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::Begin("Sandbox", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(100.f);

    if (ImGui::BeginCombo("Material", ParticleTypeNames[static_cast<int>(brush->particleType())]))
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
    guiBrushRadius = brush->radius();
    if (ImGui::SliderFloat("Brush radius", &guiBrushRadius, Brush::kMinRadius, Brush::kMaxRadius, "%.1f"))
    {
        brush->setRadius(guiBrushRadius);
    }

    ImGui::Separator();

    guiShowBrushHighlight = brush->highlight();
    if (ImGui::Checkbox("Show brush highlight", &guiShowBrushHighlight))
    {
        brush->toggleHighlight();
    }
    ImGui::Checkbox("Show controls", &guiShowControls);
    ImGui::Checkbox("Show FPS", &guiShowFPS);

    if (guiShowControls)
    {
        ImGui::Separator();
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

        if (ImGui::BeginTable("table_controls", 2, ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Action");
            ImGui::TableSetupColumn("Control");
            ImGui::TableHeadersRow();

            #define CTRL_TABLE_ENTRY(ACTION, CTRL) \
            ImGui::TableNextRow(); \
            ImGui::TableSetColumnIndex(0); \
            ImGui::Text((ACTION)); \
            ImGui::TableSetColumnIndex(1); \
            ImGui::Text((CTRL)); \

            #define CTRL_TABLE_SEPARATOR() \
            ImGui::TableNextRow(); \
            ImGui::TableSetColumnIndex(0); \
            ImGui::Separator(); \
            ImGui::TableSetColumnIndex(1); \
            ImGui::Separator();

            CTRL_TABLE_ENTRY("Draw (primary)", "Left Click");
            CTRL_TABLE_ENTRY("Draw (secondary)", "Right Click");
            CTRL_TABLE_ENTRY("Resize brush", "Scroll");
            CTRL_TABLE_ENTRY("Cycle material", "Ctrl + Scroll");
            CTRL_TABLE_ENTRY("Fill", "F");
            CTRL_TABLE_ENTRY("Clear", "R");
            CTRL_TABLE_ENTRY("Undo", "Ctrl + Z");
            
            CTRL_TABLE_SEPARATOR();

            CTRL_TABLE_ENTRY("Toggle Brush Highlight", "H");
            CTRL_TABLE_ENTRY("Toggle Show Controls", "Alt + C");
            CTRL_TABLE_ENTRY("Toggle Show FPS", "Alt + F");

            ImGui::EndTable();
        }

        ImGui::PopStyleVar();
    }
    if (guiShowFPS)
    {
        ImGui::Separator();
        ImGui::Text("FPS: %f", fps);
    }

    ImGui::PopItemWidth();
    ImGui::End();

    //////////////////
    // Draw //
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    grid->draw();

    endTime = SDL_GetPerformanceCounter();
    deltaTime = static_cast<double>(endTime - startTime) / freq;
#ifdef EMSCRIPTEN
    deltaTime = std::max(deltaTime, 0.001);
#endif
    if (deltaTime < kFrameDuration)
    {
        while (static_cast<double>(SDL_GetPerformanceCounter() - startTime) / freq < kFrameDuration) {}
        deltaTime = kFrameDuration;
    }
    fps = 1. / deltaTime;

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