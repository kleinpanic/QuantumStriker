#include "background.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define GRID_SPACING 100
#define NUM_BG_OBJECTS 150

typedef enum {
    BG_STAR,
    BG_PLANET,
    BG_HEALTH
} BGType;

typedef struct {
    BGType type;
    float x, y;
    int size;
    SDL_Color color;
} BGObject;

static BGObject bgObjects[NUM_BG_OBJECTS];
static int bgInitialized = 0;

static void init_background_objects() {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_BG_OBJECTS; i++) {
        int r = rand() % 100;
        if (r < 70) {
            bgObjects[i].type = BG_STAR;
            bgObjects[i].size = 1 + rand() % 3; // 1 to 3 pixels
            bgObjects[i].color.r = 200 + rand() % 56;
            bgObjects[i].color.g = 200 + rand() % 56;
            bgObjects[i].color.b = 200 + rand() % 56;
            bgObjects[i].color.a = 255;
        } else if (r < 90) {
            bgObjects[i].type = BG_PLANET;
            bgObjects[i].size = 40 + rand() % 40; // 40 to 80 pixels diameter
            bgObjects[i].color.r = rand() % 256;
            bgObjects[i].color.g = rand() % 256;
            bgObjects[i].color.b = rand() % 256;
            bgObjects[i].color.a = 255;
        } else {
            bgObjects[i].type = BG_HEALTH;
            bgObjects[i].size = 15;
            bgObjects[i].color.r = 0;
            bgObjects[i].color.g = 255;
            bgObjects[i].color.b = 0;
            bgObjects[i].color.a = 255;
        }
        // Position in a large world
        bgObjects[i].x = -2000 + rand() % 4000;
        bgObjects[i].y = -2000 + rand() % 4000;
    }
    bgInitialized = 1;
}

// Helper to draw a filled circle (for planets and enemies)
static void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void draw_background(SDL_Renderer* renderer, float cam_x, float cam_y, int screen_width, int screen_height) {
    // Fill with a deep-space color.
    SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);
    SDL_RenderClear(renderer);

    // Draw grid lines.
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    int world_left = (int)cam_x;
    int world_right = (int)(cam_x + screen_width);
    int world_top = (int)cam_y;
    int world_bottom = (int)(cam_y + screen_height);
    
    int start_x = world_left - (world_left % GRID_SPACING);
    for (int x = start_x; x <= world_right; x += GRID_SPACING) {
        int screen_x = x - cam_x;
        SDL_RenderDrawLine(renderer, screen_x, 0, screen_x, screen_height);
    }
    
    int start_y = world_top - (world_top % GRID_SPACING);
    for (int y = start_y; y <= world_bottom; y += GRID_SPACING) {
        int screen_y = y - cam_y;
        SDL_RenderDrawLine(renderer, 0, screen_y, screen_width, screen_y);
    }
    
    // Initialize background objects if needed.
    if (!bgInitialized)
        init_background_objects();
    
    // Draw background objects.
    for (int i = 0; i < NUM_BG_OBJECTS; i++) {
        float objScreenX = bgObjects[i].x - cam_x;
        float objScreenY = bgObjects[i].y - cam_y;
        if (objScreenX < -100 || objScreenX > screen_width + 100 ||
            objScreenY < -100 || objScreenY > screen_height + 100)
            continue;
        SDL_SetRenderDrawColor(renderer, bgObjects[i].color.r, bgObjects[i].color.g, bgObjects[i].color.b, bgObjects[i].color.a);
        if (bgObjects[i].type == BG_STAR) {
            SDL_Rect rect = { (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].size };
            SDL_RenderFillRect(renderer, &rect);
        } else if (bgObjects[i].type == BG_PLANET) {
            int radius = bgObjects[i].size / 2;
            draw_filled_circle(renderer, (int)objScreenX + radius, (int)objScreenY + radius, radius);
        } else if (bgObjects[i].type == BG_HEALTH) {
            SDL_Rect rect = { (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].size };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

