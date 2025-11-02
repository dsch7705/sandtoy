#include "SDL3/SDL_timer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>

#include <iostream>
#include <ctime>
#include <memory>
#include <fstream>
#include <filesystem>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/wasmfs.h>
#endif

#include "particle_grid.h"
#include "brush.h"
#include "util.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

// Constants //
constexpr int kGridWidth { 256 };
constexpr int kGridHeight { 128 };

int cellScale;
int screenWidth;
int screenHeight;

constexpr int kFrameCap { 240 };
constexpr double kFrameDuration { kFrameCap ? 1. / kFrameCap : -1 };
///////////////

static SDL_Window* window;
static SDL_Renderer* renderer;

static Uint64 startTime, endTime;
static double deltaTime, fps;

static ParticleGrid* grid;
static Brush* brush;
static ImGuiIO* guiIO;

static int guiBrushRadius;
static float guiBrushRotation;
static bool guiShowBrushHighlight;
static bool guiShowControls { false };
static bool guiShowFPS { true };

static bool guiShowTemperature;

static Uint64 freq = SDL_GetPerformanceFrequency();

#ifdef EMSCRIPTEN
    EM_JS(void, setupPersistentStorage, (), {
        FS.mkdir('/persistent');
        FS.mount(IDBFS, {}, '/persistent');
        FS.syncfs(true, function (err) {
            if (err) console.error("IDBFS sync (load) error: ", err);
            else console.log("IDBFS loaded from IndexedDB");
        });
    });

#define SYNC_FS() (EM_ASM({ FS.syncfs(false, () => console.log("sync")); }))
#define LOAD_FS() (EM_ASM({ FS.syncfs(true, () => console.log("load")); }))

#endif

static bool quit { false };
static void mainloop()
{
    startTime = SDL_GetPerformanceCounter();

    static bool saveModalOpened { false };
    static bool openModalOpened { false };

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
                brush->pushCanvasState();
                if (!grid->clear())
                {
                    brush->popCanvasState();
                }
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

            case SDLK_I:
                if (e.key.mod & SDL_KMOD_ALT)
                {
                    grid->toggleShowTemp();
                    break;
                }

            case SDLK_S:
                if (e.key.mod & SDL_KMOD_CTRL)
                {
                    saveModalOpened = true;
                }
                break;

            case SDLK_O:
                if (e.key.mod & SDL_KMOD_CTRL)
                {
                    openModalOpened = true;
                }
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
    brush->update();
    grid->update();
    ////////////

    // Edit Sandbox //
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Sandbox", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(100.f);

    if (ImGui::BeginCombo("Material", kParticleTypeNames[static_cast<int>(brush->particleType())].c_str()))
    {
        for (int i = 0; i < static_cast<int>(ParticleType::COUNT); ++i)
        {
            if (ImGui::Selectable(kParticleTypeNames[i].c_str()))
            {
                brush->setParticleType(static_cast<ParticleType>(i));
            }

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            ImVec2 selectableSize = ImGui::GetItemRectSize();
            float rectWidth = ImGui::GetFrameHeight() / 2;
            
            ImVec2 rectEnd = ImGui::GetItemRectMax();
            rectEnd.x -= 5;
            rectEnd.y -= (selectableSize.y - rectWidth) / 2;
            ImVec2 rectStart(rectEnd.x - rectWidth, rectEnd.y - rectWidth);

            for (int c = 0; c < 4; ++c)
            {
                ImVec2 p1(rectStart.x + (rectWidth / 2) * (c % 2), rectStart.y + (rectWidth / 2) * (c > 1));
                ImVec2 p2(p1.x + rectWidth / 2, p1.y + rectWidth / 2);

                uint32_t col = kParticleColors.at(static_cast<ParticleType>(i)).at(c);
                col = (col & 0xFF000000) >> 24 | (col & 0x00FF0000) >> 8 | (col & 0x0000FF00) << 8 | (col & 0x000000FF) << 24;
                drawList->AddRectFilled(p1, p2, col);
            }
            //drawList->AddRectFilled(rectStart, rectEnd, 0xff0000ff);
        }
        ImGui::EndCombo();
    }
    if (ImGui::BeginCombo("Brush type", BrushTypeNames[static_cast<int>(brush->brushType())]))
    {
        for (int i = 0; i < static_cast<int>(BrushType::COUNT); ++i)
        {
            if (ImGui::Selectable(BrushTypeNames[i]))
            {
                brush->setBrushType(static_cast<BrushType>(i));
            }
        }
        ImGui::EndCombo();
    }
    guiBrushRadius = brush->radius();
    if (ImGui::SliderInt("Brush radius", &guiBrushRadius, Brush::kMinRadius, Brush::kMaxRadius))
    {
        brush->setRadius(guiBrushRadius);
    }
    guiBrushRotation = brush->rotation();
    if (ImGui::SliderFloat("Brush rotation", &guiBrushRotation, 0.f, 2.f * Util::PI, "%.3f"))
    {
        brush->setRotation(guiBrushRotation);
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
            CTRL_TABLE_ENTRY("Rotate brush", "Shift + Scroll");
            CTRL_TABLE_ENTRY("Cycle material", "Ctrl + Scroll");
            CTRL_TABLE_ENTRY("Fill", "F");
            CTRL_TABLE_ENTRY("Clear", "R");
            CTRL_TABLE_ENTRY("Undo", "Ctrl + Z");
            CTRL_TABLE_ENTRY("Heat", "Middle Click");
            CTRL_TABLE_ENTRY("Cool", "Shift + Middle Click");
            
            CTRL_TABLE_SEPARATOR();

            CTRL_TABLE_ENTRY("Toggle Brush Highlight", "H");
            CTRL_TABLE_ENTRY("Toggle Infrared Mode", "Alt + I");
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
    
    // Debug //
    static float debugWindowWidth { 100.f };
    ImGui::SetNextWindowPos(ImVec2(screenWidth - debugWindowWidth, 0.f));
    ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ParticleState hoveredCellState = brush->hoveredCell() ? brush->hoveredCell()->particleState() : defaultParticleState(ParticleType::Air, grid->ambientTemperature);
    ImGui::SeparatorText("Hovered particle");
    ImGui::Text("Type: %s", kParticleTypeNames[static_cast<int>(hoveredCellState.type)].c_str());
    ImGui::Text("Phase: %s", kParticlePhaseNames[static_cast<int>(hoveredCellState.phase)].c_str());
    ImGui::Text("Temperature: %.2f", hoveredCellState.temperature);

    ImGui::SeparatorText("World");

    ImGui::PushItemWidth(debugWindowWidth / 2.f);
    if (ImGui::DragFloat("Ambient temp", &grid->ambientTemperature, 1.f, -273.f, 3000.f))
    {
        grid->ambientTemperature = std::min(std::max(grid->ambientTemperature, Util::kAbsZero), Util::kMaxTemp);
    }
    ImGui::PopItemWidth();

    guiShowTemperature = grid->showTemp();
    if (ImGui::Checkbox("Infrared mode", &guiShowTemperature))
    {
        grid->toggleShowTemp();
    }
    debugWindowWidth = ImGui::GetWindowWidth();

    // State
    if (ImGui::CollapsingHeader("State"))
    {
        const char* playBtnText = grid->isPaused ? "Play" : "Pause";
        if (ImGui::Button(playBtnText))
        {
            grid->isPaused = !grid->isPaused;
        }
    
        if (ImGui::Button("Save state"))
        {
            grid->savedCanvasStates.push_back(grid->getCanvasState());
        }
    
        if (grid->savedCanvasStates.empty()) ImGui::BeginDisabled();
        if (ImGui::Button("Load state"))
        {
            ImGui::OpenPopup("Select state");
        }
        if (grid->savedCanvasStates.empty()) ImGui::EndDisabled();
    
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(-1.f, 80));
        if (ImGui::BeginPopup("Select state", ImGuiWindowFlags_AlwaysAutoResize))
        {
            int stateToDelete { -1 };
    
            for (int i = 0; i < grid->savedCanvasStates.size(); ++i)
            {
                if (ImGui::Selectable(std::to_string(i).c_str()))
                {
                    grid->setCanvasState(grid->savedCanvasStates.at(i));
                }
    
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::Button("Delete"))
                    {
                        stateToDelete = i;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
    
            if (grid->savedCanvasStates.empty())
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
    
            if (stateToDelete >= 0)
            {
                grid->savedCanvasStates.erase(grid->savedCanvasStates.begin() + stateToDelete);
            }
        }
    }

    // Modals
    if (saveModalOpened) ImGui::OpenPopup("Save sandbox");
    if (ImGui::BeginPopupModal("Save sandbox", &saveModalOpened, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char buf[256];
        size_t bufLen = strlen(buf);

        ImGui::Text("Enter sandbox title:");
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##title", buf, sizeof(buf));
        ImGui::SameLine();

        if (bufLen == 0) ImGui::BeginDisabled();
        if (ImGui::Button("Save"))
        {
            std::filesystem::path path(SANDBOXES_DIR);
            path = path / buf;
            path.replace_extension(".snbx");

            std::ofstream o(path);
            if (o.is_open())
            {
                cereal::BinaryOutputArchive oa(o);
                oa << *grid;
                o.close();

                #ifdef EMSCRIPTEN
                SYNC_FS();
                #endif
            }
            else  
            {
                std::cerr << __func__ << ": Failed to open file" << std::endl;
            }

            buf[0] = '\0';
            saveModalOpened = false;
            ImGui::CloseCurrentPopup();
        }
        if (bufLen == 0) ImGui::EndDisabled();

        ImGui::EndPopup();
    }

    if (openModalOpened) ImGui::OpenPopup("Open sandbox");
    ImGui::SetNextWindowSize(ImVec2(175, 150));
    if (ImGui::BeginPopupModal("Open sandbox", &openModalOpened, ImGuiWindowFlags_NoResize))
    {
        
        ImGui::BeginChild("##selectFile", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_Borders);

        static const auto SENTINEL = std::filesystem::directory_entry();
        static auto selectedEntry = SENTINEL;

        auto dir_iter = std::filesystem::directory_iterator(SANDBOXES_DIR);
        for (auto& entry : dir_iter)
        {
            if (entry.is_regular_file() && entry.path().extension() == ".snbx")
            {
                if (ImGui::Selectable(entry.path().filename().string().c_str(), entry == selectedEntry))
                {
                    selectedEntry = entry;
                }
            }
        }
        
        ImGui::EndChild();

        bool invalidEntry = selectedEntry == SENTINEL;
        if (invalidEntry) ImGui::BeginDisabled();
        if (ImGui::Button("Open"))
        {
            std::ifstream i(selectedEntry.path());
            if (i.is_open())
            {
                cereal::BinaryInputArchive ia(i);
                ia >> *grid;
                i.close();
            }
            else  
            {
                std::cerr << "Failed to open file '" << selectedEntry.path().string() << "'\n";
            }

            selectedEntry = SENTINEL;
            openModalOpened = false;
            ImGui::CloseCurrentPopup();
        }
        if (invalidEntry) ImGui::EndDisabled();

        ImGui::EndPopup();
    }

    ImGui::End();
    ///////////
    // Draw //
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    grid->draw();

    endTime = SDL_GetPerformanceCounter();
    deltaTime = static_cast<double>(endTime - startTime) / freq;
#ifdef EMSCRIPTEN
    deltaTime = std::max(deltaTime, 0.001);
#endif
    if (kFrameDuration > 0 && deltaTime < kFrameDuration)
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
    // Check to make sure all particle types have a ParticleProperties
    bool missingParticleProperties = false;
    for (int i = 0; i < static_cast<int>(ParticleType::COUNT); ++i)
    {
        if (!kParticleProperties.contains(static_cast<ParticleType>(i)))
        {
            std::cerr << "[INIT] ParticleType '" << kParticleTypeNames[i] << "' does not have a ParticleProperties entry in kParticleProperties.\n";
            missingParticleProperties = true;
        }
        if (!kParticleColors.contains(static_cast<ParticleType>(i)))
        {
            std::cerr << "[INIT] ParticleType '" << kParticleTypeNames[i] << "' does not have a ParticleColors entry in kParticleColors.\n";
            missingParticleProperties = true;
        }
    }
    if (missingParticleProperties) return -1;

    // Ensure necessary directories
    #ifdef EMSCRIPTEN
    setupPersistentStorage();
    #endif
    if (!std::filesystem::exists(SANDBOXES_DIR))
    {
        std::filesystem::create_directories(SANDBOXES_DIR);

        #ifdef EMSCRIPTEN
        SYNC_FS();
        #endif
    }


    SDL_Init(SDL_INIT_VIDEO);
    std::srand(std::time(0));
    
    int displayCount;
    SDL_DisplayID* displayIDs = SDL_GetDisplays(&displayCount);
    if (displayCount == 0)
    {
        std::cerr << "No available displays" << std::endl;
        return -1;
    }

    const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(*displayIDs);
    if (!displayMode)
    {
        std::cerr << "Failed to query display mode: " << SDL_GetError() << std::endl;
        return -1;
    }

    cellScale = ((float)displayMode->w / kGridWidth) * 0.9;
    screenWidth = cellScale * kGridWidth;
    screenHeight = cellScale * kGridHeight;

    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
    window = SDL_CreateWindow("SandToy", screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // ImGui init //
    ImGui::CreateContext();
    guiIO = &ImGui::GetIO();

    guiIO->Fonts->AddFontFromFileTTF(ASSETS_DIR"/fonts/Roboto-Medium.ttf", 18.f);

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

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

    ParticleGrid::cleanup();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_Quit();
    return 0;
}
