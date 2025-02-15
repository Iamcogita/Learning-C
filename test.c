#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>

int main() {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "SDL Window",                  // Window title
        SDL_WINDOWPOS_CENTERED,        // X position
        SDL_WINDOWPOS_CENTERED,        // Y position
        800, 600,                      // Width, Height
        SDL_WINDOW_SHOWN               // Window flags
    );

    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Delay(3000);  // Keep window open for 3 seconds

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
