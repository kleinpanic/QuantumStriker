#include "enemy.h"
#include "debug.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void init_enemies(Enemy enemies[]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
    }
    DEBUG_PRINT(2, 3, "Enemies initialized: %d enemies set inactive", MAX_ENEMIES);
}

void update_enemies(Enemy enemies[], float player_x, float player_y, float difficulty) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            float diff_x = player_x - enemies[i].x;
            float diff_y = player_y - enemies[i].y;
            if (fabs(diff_x) > 2.0f)
                enemies[i].x += (diff_x > 0 ? 0.5f : -0.5f) * difficulty;
            if (fabs(diff_y) > 2.0f)
                enemies[i].y += (diff_y > 0 ? 0.3f : -0.3f) * difficulty;
            DEBUG_PRINT(3, 2, "Updated enemy %d: new position = (%.2f, %.2f)", i, enemies[i].x, enemies[i].y);
        }
    }
}

void draw_enemies(Enemy enemies[], SDL_Renderer* renderer, float cam_x, float cam_y) {
    int drawn = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            int cx = (int)(enemies[i].x - cam_x);
            int cy = (int)(enemies[i].y - cam_y);
            filledEllipseRGBA(renderer, cx, cy, 15, 10, 255, 0, 0, 255);
            drawn++;
        }
    }
    DEBUG_PRINT(3, 2, "Drawn %d active enemies", drawn);
}

void spawn_enemy(Enemy enemies[], float player_x, float player_y, float spawnRateModifier) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            float angle = (rand() % 360) * (M_PI / 180.0f);
            float distance = 150 + rand() % 150;  // 150 to 300 units away
            enemies[i].x = player_x + cosf(angle) * distance;
            enemies[i].y = player_y + sinf(angle) * distance;
            // Scale enemy health by spawnRateModifier.
            enemies[i].health = (int)(3 * spawnRateModifier);
            enemies[i].type = 1;
            enemies[i].active = 1;
            DEBUG_PRINT(3, 3, "Spawned enemy at (%.2f, %.2f) with health %d", enemies[i].x, enemies[i].y, enemies[i].health);
            break;
        }
    }
}

