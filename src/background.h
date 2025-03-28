#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>
#include "debug.h"
#include "config.h"

/* Expanded galactic object types */
typedef enum {
    BG_STAR,
    BG_PLANET,
    BG_MOON,
    BG_ASTEROID,
    BG_NEUTRON_STAR,
    BG_GALAXY,
    BG_NEBULA,
    BG_STAR_CLUSTER,
    BG_BLACKHOLE
} BGType;

/* Structure for a background object */
typedef struct {
    BGType type;
    float x, y;   // world coordinates
    int size;     // general size or diameter
    SDL_Color color;
} BGObject;


void draw_background(SDL_Renderer* renderer, float cam_x, float cam_y, int screen_width, int screen_height);

#endif

