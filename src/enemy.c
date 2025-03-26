#include "enemy.h"
#include "debug.h"
#include "bullet.h"  // For shooting
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper function for enemy shooter to fire at the player.
void enemy_shoot(Enemy *enemy, BulletPool *pool, float player_x, float player_y) {
    float dx = player_x - enemy->x;
    float dy = player_y - enemy->y;
    float angle = atan2f(dy, dx) * 180.0f / M_PI + 90.0f; // adjust so that 0° is up
    shoot_bullet(pool, enemy->x, enemy->y, angle, 1); // enemy bullet
    DEBUG_PRINT(3, 2, "Enemy of type SHOOTER fired bullet towards player at angle %.2f", angle);
}

// Initialize all enemies and their additional fields.
void init_enemies(Enemy enemies[]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemies[i].timer = 0;
        enemies[i].shootTimer = 0;
        enemies[i].visible = 1;
        enemies[i].type = ENEMY_BASIC;  // default type
    }
    DEBUG_PRINT(2, 3, "Enemies initialized: %d enemies set inactive", MAX_ENEMIES);
}

// Update enemies with different behaviors based on type.
void update_enemies(Enemy enemies[], float player_x, float player_y, float difficulty) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active)
            continue;

        float diff_x = player_x - enemies[i].x;
        float diff_y = player_y - enemies[i].y;
        float distance = sqrtf(diff_x * diff_x + diff_y * diff_y);
        float moveStep;
        // Increase the timer for enemy AI
        enemies[i].timer++;

        switch (enemies[i].type) {
            case ENEMY_BASIC:
                moveStep = 0.5f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            case ENEMY_SHOOTER:
                moveStep = 0.5f * difficulty;
                if (distance < 75) {
                    if (fabs(diff_x) > 2.0f)
                        enemies[i].x -= (diff_x > 0 ? moveStep : -moveStep);
                    if (fabs(diff_y) > 2.0f)
                        enemies[i].y -= (diff_y > 0 ? moveStep : -moveStep);
                } else {
                    // Otherwise approach the player
                    if (fabs(diff_x) > 2.0f)
                        enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                    if (fabs(diff_y) > 2.0f)
                        enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                }

                // If within 300 units and shootTimer has expired, shoot at player.
                if (distance < 300 && enemies[i].shootTimer <= 0) {
                    // In a real implementation, the bullet pool pointer should be passed in;
                    // For now, we assume the game loop calls enemy_shoot() with access to the bullet pool.
                    // Here we just set shootTimer to a reset value.
                    enemies[i].shootTimer = 120; // reset timer
                    // Actual shooting will be handled in game_loop (see below)
                } else if (enemies[i].shootTimer > 0) {
                    enemies[i].shootTimer--;
                }
                break;
            case ENEMY_TANK:
                moveStep = 0.3f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            case ENEMY_EVASIVE:
                moveStep = 0.5f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep) + ((rand() % 3 - 1) * 0.3f);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep) + ((rand() % 3 - 1) * 0.3f);
                break;
            case ENEMY_FAST:
                moveStep = 0.8f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            case ENEMY_SPLITTER:
                moveStep = 0.5f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            case ENEMY_STEALTH:
                moveStep = 0.4f * difficulty;
                // Toggle visibility every 180 frames.
                if (enemies[i].timer % 180 < 90)
                    enemies[i].visible = 1;
                else
                    enemies[i].visible = 0;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            case ENEMY_BOSS1:
                moveStep = 0.4f * difficulty;
                enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                enemies[i].x += sinf(enemies[i].timer * 0.05f) * 5.0f;
                break;
            case ENEMY_BOSS2:
                if (enemies[i].timer % 240 < 30) {
                    moveStep = 1.2f * difficulty;
                } else {
                    moveStep = 0.4f * difficulty;
                }
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            case ENEMY_BOSS3:
                moveStep = 0.5f * difficulty;
                enemies[i].x += (diff_x > 0 ? moveStep : -moveStep) + ((rand() % 5 - 2) * 0.5f);
                enemies[i].y += (diff_y > 0 ? moveStep : -moveStep) + ((rand() % 5 - 2) * 0.5f);
                break;
            default:
                moveStep = 0.5f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
        }
        DEBUG_PRINT(3, 2, "Updated enemy (type %d) at (%.2f, %.2f), distance=%.2f", enemies[i].type, enemies[i].x, enemies[i].y, distance);
    }
}

// Draw enemies with different shapes/colors based on type.
void draw_enemies(Enemy enemies[], SDL_Renderer* renderer, float cam_x, float cam_y) {
    int drawn = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active)
            continue;
        if (enemies[i].type == ENEMY_STEALTH && !enemies[i].visible)
            continue;

        int cx = (int)(enemies[i].x - cam_x);
        int cy = (int)(enemies[i].y - cam_y);

        switch (enemies[i].type) {
            case ENEMY_BASIC:
                filledEllipseRGBA(renderer, cx, cy, 15, 10, 255, 0, 0, 255);
                break;
            case ENEMY_SHOOTER:
                SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
                {
                    SDL_Rect rect = { cx - 12, cy - 8, 24, 16 };
                    SDL_RenderFillRect(renderer, &rect);
                }
                break;
            case ENEMY_TANK:
                filledEllipseRGBA(renderer, cx, cy, 20, 14, 0, 0, 255, 255);
                break;
            case ENEMY_EVASIVE: {
                Sint16 vx[3] = { cx, cx - 10, cx + 10 };
                Sint16 vy[3] = { cy - 12, cy + 8, cy + 8 };
                filledPolygonRGBA(renderer, vx, vy, 3, 0, 255, 0, 255);
                break;
            }
            case ENEMY_FAST:
                filledEllipseRGBA(renderer, cx, cy, 10, 7, 255, 255, 0, 255);
                break;
            case ENEMY_SPLITTER:
                filledCircleRGBA(renderer, cx, cy, 12, 128, 0, 128, 255);
                aacircleRGBA(renderer, cx, cy, 12, 255, 255, 255, 255);
                break;
            case ENEMY_STEALTH:
                filledEllipseRGBA(renderer, cx, cy, 15, 10, 192, 192, 192, 255);
                break;
            case ENEMY_BOSS1:
                filledEllipseRGBA(renderer, cx, cy, 30, 20, 255, 0, 255, 255);
                break;
            case ENEMY_BOSS2:
                filledEllipseRGBA(renderer, cx, cy, 28, 18, 0, 255, 255, 255);
                break;
            case ENEMY_BOSS3:
                filledEllipseRGBA(renderer, cx, cy, 32, 22, 200, 0, 0, 255);
                aacircleRGBA(renderer, cx, cy, 32, 255, 255, 255, 255);
                break;
            default:
                filledEllipseRGBA(renderer, cx, cy, 15, 10, 255, 0, 0, 255);
                break;
        }
        drawn++;
    }
    DEBUG_PRINT(3, 2, "Drawn %d active enemies", drawn);
}

// Spawn an enemy with type selected based on the current score.
void spawn_enemy(Enemy enemies[], float player_x, float player_y, int score) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            float angle = (rand() % 360) * (M_PI / 180.0f);
            float distance = 150 + rand() % 150;  // 150 to 300 units away
            enemies[i].x = player_x + cosf(angle) * distance;
            enemies[i].y = player_y + sinf(angle) * distance;
            int r = rand() % 100;
            // Determine enemy type based on score thresholds.
            if (score < 100) {
                enemies[i].type = ENEMY_BASIC;
            } else if (score < 500) {
                enemies[i].type = (r < 80) ? ENEMY_BASIC : ENEMY_SHOOTER;
            } else if (score < 1000) {
                if (r < 50)
                    enemies[i].type = ENEMY_BASIC;
                else if (r < 75)
                    enemies[i].type = ENEMY_SHOOTER;
                else
                    enemies[i].type = ENEMY_TANK;
            } else if (score < 2000) {
                if (r < 40)
                    enemies[i].type = ENEMY_BASIC;
                else if (r < 60)
                    enemies[i].type = ENEMY_SHOOTER;
                else if (r < 75)
                    enemies[i].type = ENEMY_TANK;
                else if (r < 85)
                    enemies[i].type = ENEMY_FAST;
                else if (r < 95)
                    enemies[i].type = ENEMY_EVASIVE;
                else
                    enemies[i].type = ENEMY_SPLITTER;
            } else { // score >= 2000
                if (r < 30)
                    enemies[i].type = ENEMY_BASIC;
                else if (r < 40)
                    enemies[i].type = ENEMY_SHOOTER;
                else if (r < 50)
                    enemies[i].type = ENEMY_TANK;
                else if (r < 60)
                    enemies[i].type = ENEMY_FAST;
                else if (r < 70)
                    enemies[i].type = ENEMY_EVASIVE;
                else if (r < 75)
                    enemies[i].type = ENEMY_SPLITTER;
                else if (r < 80)
                    enemies[i].type = ENEMY_STEALTH;
                else if (r < 85)
                    enemies[i].type = ENEMY_BOSS1;
                else if (r < 90)
                    enemies[i].type = ENEMY_BOSS2;
                else
                    enemies[i].type = ENEMY_BOSS3;
            }

            // Set health based on type.
            switch (enemies[i].type) {
                case ENEMY_BASIC: enemies[i].health = 3; break;
                case ENEMY_SHOOTER: enemies[i].health = 3; enemies[i].shootTimer = 120; break;
                case ENEMY_TANK: enemies[i].health = 6; break;
                case ENEMY_EVASIVE: enemies[i].health = 3; break;
                case ENEMY_FAST: enemies[i].health = 2; break;
                case ENEMY_SPLITTER: enemies[i].health = 3; break;
                case ENEMY_STEALTH: enemies[i].health = 3; break;
                case ENEMY_BOSS1: enemies[i].health = 20; break;
                case ENEMY_BOSS2: enemies[i].health = 25; break;
                case ENEMY_BOSS3: enemies[i].health = 30; break;
                default: enemies[i].health = 3; break;
            }
            enemies[i].active = 1;
            enemies[i].timer = 0;
            enemies[i].visible = 1;
            DEBUG_PRINT(3, 3, "Spawned enemy type %d at (%.2f, %.2f) with health %d", 
                        enemies[i].type, enemies[i].x, enemies[i].y, enemies[i].health);
            break;
        }
    }
}

