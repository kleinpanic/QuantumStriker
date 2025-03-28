#ifndef MENUS_H
#define MENUS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Displays the pause menu overlay. Returns 1 if the game should resume, or 0 if it should quit.
int pause_menu(SDL_Renderer *renderer, TTF_Font *font, int screen_width, int screen_height);

#endif // MENUS_H

