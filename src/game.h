#ifndef GAME_H
#define GAME_H

#include "debug.h"
#include "score.h"
#include <SDL2/SDL.h>

typedef struct {
    float x, y;
    float radius;
    int lifetime; // frames remaining
} Explosion;
#define MAX_EXPLOSIONS 50

int get_user_top_score(const char *username, ScoreBlock *topBlock);

void game_loop();

#endif

