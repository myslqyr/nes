#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include <stdbool.h>

void sdl_init();
bool sdl_poll_quit();
void sdl_shutdown();

#endif // SDL_H
