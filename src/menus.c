#include "menus.h"
#include "debug.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

int pause_menu(SDL_Renderer *renderer, TTF_Font *font, int screen_width, int screen_height) {
    int resume = 0;
    int quit = 0;
    SDL_Event e;

    // Render a simple pause overlay.
    while (!resume && !quit) {
        // Draw translucent overlay.
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // semi-transparent black
        SDL_Rect overlay = {0, 0, screen_width, screen_height};
        SDL_RenderFillRect(renderer, &overlay);

        // Render pause text.
        // (For simplicity, hard-code the messages.)
        SDL_Color white = {255, 255, 255, 255};
        // Example texts.
        // You could later add options and highlight them.
        // Here, "Resume" and "Quit" are options.
        // Render centered.
        // (In a full implementation, you might use proper layout.)
        // Draw "PAUSED"
        // (Assuming render_text is defined in your code base.)
        extern void render_text(SDL_Renderer*, TTF_Font*, int, int, const char*, SDL_Color);
        render_text(renderer, font, screen_width/2 - 50, screen_height/2 - 80, "PAUSED", white);
        render_text(renderer, font, screen_width/2 - 70, screen_height/2 - 40, "Press Q/Escape to Resume", white);
        render_text(renderer, font, screen_width/2 - 50, screen_height/2, "Press X to Quit", white);

        SDL_RenderPresent(renderer);

        // Process events.
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q) {
                    resume = 1;
                } else if (e.key.keysym.sym == SDLK_x) {
                    quit = 1;
                }
            }
        }
        SDL_Delay(16);
    }
    return resume ? 1 : 0;
}

