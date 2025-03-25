#ifndef ENEMY_H
#define ENEMY_H

#define MAX_ENEMIES 100

#include <SDL2/SDL.h>

typedef struct {
    float x, y;
    int health;
    int type;   // For future extension (different enemy AIs)
    int active;
} Enemy;

void init_enemies(Enemy enemies[]);
void update_enemies(Enemy enemies[], float player_x, float player_y, float difficulty);
void draw_enemies(Enemy enemies[], SDL_Renderer* renderer, float cam_x, float cam_y);
void spawn_enemy(Enemy enemies[], float player_x, float player_y, float spawnRateModifier);

#endif

