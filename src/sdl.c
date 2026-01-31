#include "../include/sdl.h"

static SDL_Window *window = NULL;

void sdl_init() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("NESemulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 240, SDL_WINDOW_SHOWN);
}

bool sdl_poll_quit() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
        }
    }
    return false;
}

void sdl_shutdown() {
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}
