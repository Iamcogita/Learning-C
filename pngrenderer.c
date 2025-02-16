#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

int main(int argc, char *argv[]) {

    SDL_SetMainReady(); // Ensures SDL knows we are handling main ourselves

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("PNG Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    IMG_Init(IMG_INIT_PNG);
    SDL_Surface *imageSurface = IMG_Load("test-img.png");
    if (!imageSurface) {
        printf("Unable to load image! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    int running = 1;
    SDL_Event event; 
    int colorToggle = 0; // Toggle variable for background color

    while (running) {
        while (SDL_PollEvent(&event)) {
            // Closing the window manually
            if (event.type == SDL_QUIT ) { 
                running = 0; 
            } 
            // Key pressed is ESCAPE key
            if (event.type == SDL_KEYDOWN) { 
                if (event.key.keysym.sym == SDLK_ESCAPE) { 
                    running = 0;
                } 
            }
            // Key pressed is SPACE key
            if (event.type == SDL_KEYDOWN) { 
                if (event.key.keysym.sym == SDLK_SPACE) {
                    colorToggle = !colorToggle ; // Toggle color
                }
            }
            // Change background color
            if (colorToggle) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue
            }
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}