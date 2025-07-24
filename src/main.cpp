#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


// Constants //
constexpr int kScreenWidth { 640 };
constexpr int kScreenHeight { 480 };
///////////////

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    SDL_Window *pWindow = SDL_CreateWindow("SandToy", kScreenWidth, kScreenHeight, 0);
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, nullptr);

    bool quit = false;
    while (!quit)
    {
        // Handle Events //
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_EVENT_QUIT:
                quit = true;
                break;

            default:
                break;

            }
        }
        ///////////////////
        // Draw //
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);
        SDL_RenderPresent(pRenderer);
        //////////
    }
    
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    
    SDL_Quit();
    return 0;
}