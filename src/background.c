#include "background.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Internal storage for background objects */
static BGObject bgObjects[NUM_BG_OBJECTS];
static int bgInitialized = 0;

/* Helper: check if two circles (centered at (x,y) with radius r) overlap */
static int overlaps(float x, float y, int size, float x2, float y2, int size2) {
    float dx = x - x2;
    float dy = y - y2;
    float distance = sqrtf(dx * dx + dy * dy);
    return (distance < (size/2 + size2/2 + 5)); // add a small margin (5 pixels)
}

/* Helper: Draw a radial gradient circle.
 * Approximates a gradient by drawing multiple concentric circles.
 * inner: center color, outer: edge color.
 */
static void draw_radial_gradient(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color inner, SDL_Color outer) {
    for (int r = radius; r > 0; r--) {
        float factor = (float)r / radius;
        SDL_Color color;
        color.r = outer.r + (int)((inner.r - outer.r) * factor);
        color.g = outer.g + (int)((inner.g - outer.g) * factor);
        color.b = outer.b + (int)((inner.b - outer.b) * factor);
        color.a = outer.a + (int)((inner.a - outer.a) * factor);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (int angle = 0; angle < 360; angle += 5) {
            float rad = angle * (M_PI / 180.0f);
            int x = cx + r * cosf(rad);
            int y = cy + r * sinf(rad);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

/* Basic filled circle for non-gradient effects */
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

/* Drawing routines for each BGType */
static void draw_bg_star(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    /* Twinkle effect by slightly varying brightness */
    int flicker = rand() % 30;
    SDL_Color modColor = { 
        (Uint8)fmin(255, color.r + flicker), 
        (Uint8)fmin(255, color.g + flicker), 
        (Uint8)fmin(255, color.b + flicker), 
        color.a 
    };
    SDL_Rect rect = { x, y, size, size };
    SDL_SetRenderDrawColor(renderer, modColor.r, modColor.g, modColor.b, modColor.a);
    SDL_RenderFillRect(renderer, &rect);
}

static void draw_bg_planet(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    int radius = size / 2;
    SDL_Color inner = { (Uint8)fmin(255, color.r + 50), (Uint8)fmin(255, color.g + 50), (Uint8)fmin(255, color.b + 50), 255 };
    draw_radial_gradient(renderer, x + radius, y + radius, radius, inner, color);
}

static void draw_bg_moon(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    int radius = size / 2;
    draw_filled_circle(renderer, x + radius, y + radius, radius);
    SDL_Color dark = { color.r / 2, color.g / 2, color.b / 2, color.a };
    SDL_SetRenderDrawColor(renderer, dark.r, dark.g, dark.b, dark.a);
    draw_filled_circle(renderer, x + radius - radius/3, y + radius - radius/3, radius/3);
}

static void draw_bg_asteroid(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    int radius = size / 2;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    draw_filled_circle(renderer, x + radius, y + radius, radius);
}

static void draw_bg_neutron_star(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    (void)color;  // fixed style: always bright white
    int radius = size / 2;
    draw_filled_circle(renderer, x + radius, y + radius, radius);
}

static void draw_bg_galaxy(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    int radius = size / 2;
    SDL_Color inner = { color.r, color.g, color.b, 255 };
    SDL_Color outer = { color.r, color.g, color.b, 0 };
    draw_radial_gradient(renderer, x + radius, y + radius, radius + 10, inner, outer);
    draw_filled_circle(renderer, x + radius, y + radius, radius);
}

static void draw_bg_nebula(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    int radius = size / 2;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Color inner = { color.r, color.g, color.b, 180 };
    SDL_Color outer = { color.r, color.g, color.b, 0 };
    draw_radial_gradient(renderer, x + radius, y + radius, radius, inner, outer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

static void draw_bg_star_cluster(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    int clusterSize = size;
    for (int i = 0; i < 8; i++) {
        int offsetX = rand() % clusterSize - clusterSize/2;
        int offsetY = rand() % clusterSize - clusterSize/2;
        int starSize = 2 + rand() % 3;
        SDL_Color starInner = { color.r, color.g, color.b, 255 };
        SDL_Color starOuter = { color.r, color.g, color.b, 0 };
        draw_radial_gradient(renderer, x + offsetX, y + offsetY, starSize, starInner, starOuter);
    }
}

static void draw_bg_blackhole(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    (void)color;  // fixed style for blackhole
    int radius = size / 2;
    SDL_Color diskInner = { 255, 140, 0, 255 };
    SDL_Color diskOuter = { 255, 140, 0, 0 };
    draw_radial_gradient(renderer, x + radius, y + radius, radius + 5, diskInner, diskOuter);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    draw_filled_circle(renderer, x + radius, y + radius, radius);
}

/* Initialize background objects without overlapping.
   Each object is placed randomly within the world boundaries defined by WORLD_BORDER.
   WORLD_BORDER is assumed to be defined in config.h (default 10000).
*/
static void init_background_objects() {
    DEBUG_PRINT(3, 2, "Initializing background objects...");
    srand((unsigned int)time(NULL));
    int i = 0;
    int maxAttempts = 100;
    while (i < NUM_BG_OBJECTS) {
        BGObject obj;
        int r = rand() % 100;
        if (r < 40) {
            obj.type = BG_STAR;
            obj.size = 1 + rand() % 3;
            obj.color.r = 200 + rand() % 56;
            obj.color.g = 200 + rand() % 56;
            obj.color.b = 200 + rand() % 56;
            obj.color.a = 255;
        } else if (r < 55) {
            obj.type = BG_PLANET;
            obj.size = 40 + rand() % 40;
            obj.color.r = rand() % 256;
            obj.color.g = rand() % 256;
            obj.color.b = rand() % 256;
            obj.color.a = 255;
        } else if (r < 65) {
            obj.type = BG_MOON;
            obj.size = 20 + rand() % 20;
            obj.color.r = 180 + rand() % 76;
            obj.color.g = 180 + rand() % 76;
            obj.color.b = 180 + rand() % 76;
            obj.color.a = 255;
        } else if (r < 75) {
            obj.type = BG_ASTEROID;
            obj.size = 15 + rand() % 15;
            obj.color.r = 100 + rand() % 156;
            obj.color.g = 100 + rand() % 156;
            obj.color.b = 100 + rand() % 156;
            obj.color.a = 255;
        } else if (r < 80) {
            obj.type = BG_NEUTRON_STAR;
            obj.size = 8 + rand() % 5;
            obj.color.r = 255;
            obj.color.g = 255;
            obj.color.b = 255;
            obj.color.a = 255;
        } else if (r < 85) {
            obj.type = BG_GALAXY;
            obj.size = 80 + rand() % 40;
            obj.color.r = rand() % 256;
            obj.color.g = rand() % 256;
            obj.color.b = rand() % 256;
            obj.color.a = 200;
        } else if (r < 90) {
            obj.type = BG_NEBULA;
            obj.size = 100 + rand() % 50;
            obj.color.r = rand() % 256;
            obj.color.g = rand() % 256;
            obj.color.b = rand() % 256;
            obj.color.a = 150;
        } else if (r < 95) {
            obj.type = BG_STAR_CLUSTER;
            obj.size = 30 + rand() % 20;
            obj.color.r = 200 + rand() % 56;
            obj.color.g = 200 + rand() % 56;
            obj.color.b = 200 + rand() % 56;
            obj.color.a = 255;
        } else {
            obj.type = BG_BLACKHOLE;
            obj.size = 50 + rand() % 30;
            obj.color.r = 0;
            obj.color.g = 0;
            obj.color.b = 0;
            obj.color.a = 255;
        }
        /* Generate a random position within the world boundaries.
           WORLD_BORDER should be defined in config.h (default 10000).
           We use a coordinate system centered at 0 (from -WORLD_BORDER/2 to WORLD_BORDER/2).
        */
        obj.x = -(WORLD_BORDER / 2) + rand() % WORLD_BORDER;
        obj.y = -(WORLD_BORDER / 2) + rand() % WORLD_BORDER;
        
        /* Check for overlap with already placed objects. */
        int attempts = 0;
        int conflict = 0;
        do {
            conflict = 0;
            for (int j = 0; j < i; j++) {
                if (overlaps(obj.x, obj.y, obj.size, bgObjects[j].x, bgObjects[j].y, bgObjects[j].size)) {
                    conflict = 1;
                    obj.x = -(WORLD_BORDER / 2) + rand() % WORLD_BORDER;
                    obj.y = -(WORLD_BORDER / 2) + rand() % WORLD_BORDER;
                    break;
                }
            }
            attempts++;
        } while (conflict && attempts < maxAttempts);
        
        bgObjects[i] = obj;
        i++;
    }
    bgInitialized = 1;
    DEBUG_PRINT(3, 3, "Initialized %d background objects", NUM_BG_OBJECTS);
}

/* Main draw function.
   If ENABLE_GRID is true, draws grid lines over the background.
   Then draws each background object using its dedicated drawing routine.
*/
void draw_background(SDL_Renderer* renderer, float cam_x, float cam_y, int screen_width, int screen_height) {
    /* Fill with deep-space background color. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);
    SDL_RenderClear(renderer);
    
#if ENABLE_GRID
    /* Draw grid lines. */
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    int world_left = (int)cam_x;
    int world_right = (int)(cam_x + screen_width);
    int world_top = (int)cam_y;
    int world_bottom = (int)(cam_y + screen_height);
    
    int start_x = world_left - (world_left % 100);
    for (int x = start_x; x <= world_right; x += 100) {
        int screen_x = x - cam_x;
        SDL_RenderDrawLine(renderer, screen_x, 0, screen_x, screen_height);
    }
    int start_y = world_top - (world_top % 100);
    for (int y = start_y; y <= world_bottom; y += 100) {
        int screen_y = y - cam_y;
        SDL_RenderDrawLine(renderer, 0, screen_y, screen_width, screen_y);
    }
#endif

    if (!bgInitialized) {
        DEBUG_PRINT(3, 2, "Background objects not initialized, initializing now.");
        init_background_objects();
    }
    
    /* Draw each background object if it is within the view (with margin) */
    for (int i = 0; i < NUM_BG_OBJECTS; i++) {
        float objScreenX = bgObjects[i].x - cam_x;
        float objScreenY = bgObjects[i].y - cam_y;
        if (objScreenX < -150 || objScreenX > screen_width + 150 ||
            objScreenY < -150 || objScreenY > screen_height + 150)
            continue;
        switch (bgObjects[i].type) {
            case BG_STAR:
                draw_bg_star(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            case BG_PLANET:
                draw_bg_planet(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            case BG_MOON:
                draw_bg_moon(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            case BG_ASTEROID:
                draw_bg_asteroid(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            case BG_NEUTRON_STAR:
                draw_bg_neutron_star(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            case BG_GALAXY:
                draw_bg_galaxy(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            case BG_NEBULA:
                draw_bg_nebula(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            case BG_STAR_CLUSTER:
                draw_bg_star_cluster(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            case BG_BLACKHOLE:
                draw_bg_blackhole(renderer, (int)objScreenX, (int)objScreenY, bgObjects[i].size, bgObjects[i].color);
                break;
            default:
                break;
        }
    }
}
