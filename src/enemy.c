#include "enemy.h"
#include "debug.h"
#include "bullet.h" 
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <math.h>

static float get_collision_radius(EnemyType type) {
    switch (type) {
        case ENEMY_BASIC:     return 15.0f;
        case ENEMY_SHOOTER:   return 15.0f;
        case ENEMY_TANK:      return 20.0f;
        case ENEMY_EVASIVE:   return 15.0f;
        case ENEMY_FAST:      return 10.0f;
        case ENEMY_SPLITTER:  return 12.0f;
        case ENEMY_STEALTH:   return 15.0f;
        case ENEMY_BOSS1:     return 30.0f;
        case ENEMY_BOSS2:     return 28.0f;
        case ENEMY_BOSS3:     return 32.0f;
        default:              return 15.0f;
    }
}

void enemy_shoot(Enemy *enemy, BulletPool *pool, float player_x, float player_y) {
    float dx = player_x - enemy->x;
    float dy = player_y - enemy->y;
    float angle = atan2f(dy, dx) * 180.0f / M_PI;
    shoot_bullet(pool, enemy->x, enemy->y, angle, 1); // enemy bullet
    DEBUG_PRINT(3, 2, "Enemy SHOOTER fired bullet towards player at angle %.2f", angle);
}

// Initialize all enemies and their additional fields.
void init_enemies(Enemy enemies[]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
        enemies[i].timer = 0;
        enemies[i].shootTimer = 0;
        enemies[i].visible = 1;
        enemies[i].type = ENEMY_BASIC;  // default type
        // Initialize angle so that by default it faces right (0° means to the right)
        enemies[i].angle = 0.0f;
    }
    DEBUG_PRINT(2, 3, "Enemies initialized: %d enemies set inactive", MAX_ENEMIES);
}

// Update enemies with different behaviors based on type.
void update_enemies(Enemy enemies[], float player_x, float player_y, float difficulty, BulletPool* pool) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active)
            continue;

        float diff_x = player_x - enemies[i].x;
        float diff_y = player_y - enemies[i].y;
        float distance = sqrtf(diff_x * diff_x + diff_y * diff_y);
        float moveStep;
        // Increase the timer for enemy AI.
        enemies[i].timer++;

        switch (enemies[i].type) {
            case ENEMY_BASIC:
            {
                moveStep = 0.5f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            }
            case ENEMY_SHOOTER:
            {
                const float desiredMin = 150.0f;
                const float desiredMax = 300.0f;
                const float targetDistance = (desiredMin + desiredMax) / 2.0f; // ~225 units
                float distanceError = distance - targetDistance;
    
                // Calculate the desired angle directly toward the player.
                float desiredAngle = atan2f(diff_y, diff_x) * 180.0f / M_PI;
    
                // Smooth rotation toward the desired angle.
                float rotationSpeed = 5.0f; // degrees per frame
                float angleDifference = desiredAngle - enemies[i].angle;
                while (angleDifference > 180.0f) angleDifference -= 360.0f;
                while (angleDifference < -180.0f) angleDifference += 360.0f;
    
                if (fabs(angleDifference) < rotationSpeed)
                    enemies[i].angle = desiredAngle;
                else
                    enemies[i].angle += (angleDifference > 0 ? rotationSpeed : -rotationSpeed);
    
                // Position adjustment to maintain target distance.
float adjustment = 0.05f * fabs(distanceError) * difficulty;
                float norm = (distance > 0) ? distance : 1.0f;
                float unit_dx = diff_x / norm;
                float unit_dy = diff_y / norm;
                if (distance > targetDistance) {
                    enemies[i].x += unit_dx * adjustment;
                    enemies[i].y += unit_dy * adjustment;
                } else {
                    enemies[i].x -= unit_dx * adjustment;
                    enemies[i].y -= unit_dy * adjustment;
                }
    
                // Add slight random lateral movement for unpredictability.
                enemies[i].x += ((rand() % 3) - 1) * 0.2f * difficulty;
                enemies[i].y += ((rand() % 3) - 1) * 0.2f * difficulty;
    
                // Shooting: fire only if within range, cooldown expired, and nearly aligned.
                if (distance < desiredMax && enemies[i].shootTimer <= 0 && fabs(angleDifference) < 5.0f) {
                    shoot_bullet(pool, enemies[i].x, enemies[i].y, enemies[i].angle - 180, 1);
                    DEBUG_PRINT(3, 2, "Rotating shooter fired bullet at angle %.2f, distance: %.2f", enemies[i].angle, distance);
                    enemies[i].shootTimer = 90;
                } else if (enemies[i].shootTimer > 0) {
                    enemies[i].shootTimer--;
                }
                break;
            }
            case ENEMY_TANK:
            {
                // Slow enemy that rotates toward the player.
                float desiredAngle = atan2f(diff_y, diff_x) * 180.0f / M_PI;
                float rotationSpeed = 2.5f;  // Slower rotation than shooter.
                float angleDifference = desiredAngle - enemies[i].angle;
                while (angleDifference > 180.0f) angleDifference -= 360.0f;
                while (angleDifference < -180.0f) angleDifference += 360.0f;
                if (fabs(angleDifference) < rotationSpeed)
                    enemies[i].angle = desiredAngle;
                else
                    enemies[i].angle += (angleDifference > 0 ? rotationSpeed : -rotationSpeed);
    
                // Move slowly in the direction the tank is facing.
                moveStep = 0.3f * difficulty;
                float rad = enemies[i].angle * (M_PI / 180.0f);
                enemies[i].x += cosf(rad) * moveStep;
                enemies[i].y += sinf(rad) * moveStep;
                
                enemies[i].x += ((rand() % 3) - 1) * 0.2f * difficulty;
                enemies[i].y += ((rand() % 3) - 1) * 0.2f * difficulty;
                break;
            }
            case ENEMY_EVASIVE:
            {
                DEBUG_PRINT(0, 2, "Spawned Evasive");
                const float bullet_danger_distance = 50.0f;
                const float bullet_repulsion_factor = 3.0f;
                // Base attraction toward the player.
                float att_x = diff_x / (distance > 0 ? distance : 1);
                float att_y = diff_y / (distance > 0 ? distance : 1);
    
                // Bullet avoidance: scan for nearby player bullets.
                float repulsion_x = 0.0f, repulsion_y = 0.0f;
                int playerBulletTooClose = 0;
                for (int i = 0; i < pool->count; i++) {
                    if (pool->bullets[i].active && pool->bullets[i].isEnemy == 0) {
                        float bx = pool->bullets[i].x - enemies[i].x;
                        float by = pool->bullets[i].y - enemies[i].y;
                        float bdist = sqrtf(bx * bx + by * by);
                        DEBUG_PRINT(0, 2, "player Bullet %d: bdist=%.2f", i, bdist);
                        if (bdist < bullet_danger_distance) {
                            playerBulletTooClose = 1;
                            repulsion_x -= bx / bdist * (bullet_danger_distance - bdist) * bullet_repulsion_factor;
                            repulsion_y -= by / bdist * (bullet_danger_distance - bdist) * bullet_repulsion_factor;
                            DEBUG_PRINT(0, 2, "Player Bullet %d: Repulsive added (%.2f, %.2f)", i, repulsion_x, repulsion_y);
                        }
                    }
                }
                // Combine attraction and repulsion.
                float final_x = att_x + repulsion_x;
                float final_y = att_y + repulsion_y;
                float final_norm = sqrtf(final_x * final_x + final_y * final_y);
                if (final_norm > 0) {
                    final_x /= final_norm;
                    final_y /= final_norm;
                }

                if (playerBulletTooClose) {
                    DEBUG_PRINT(0, 2, "Player bullets are dangerously close");
                    float flee_angle = atan2f(repulsion_y, repulsion_x) * (180.0f / M_PI) + 90.0f;
                    float angle_adjust = flee_angle - enemies[i].angle;
                    while (angle_adjust > 180.0f) angle_adjust -= 360.0f;
                    while (angle_adjust < -180.0f) angle_adjust += 360.0f;
                    DEBUG_PRINT(0, 2, "Bullet Evasion: flee_angle=%.2f, angle_adjust=%.2f", flee_angle, angle_adjust);
                    if (fabs(angle_adjust) < 5.0f)
                        enemies[i].angle = flee_angle;
                    else if (angle_adjust > 0)
                        enemies[i].angle += 5.0f;
                    else
                        enemies[i].angle -= 5.0f;
                    DEBUG_PRINT(0, 3, "Bullet Evasion: new angle=%.2f", enemies[i].angle);
                    moveStep = 0.8f * difficulty;
                    enemies[i].x -= final_x * moveStep;
                    enemies[i].y -= final_y * moveStep;
                    return; // Do not continue offensive behavior while bullets are too close.
                }

                // Evasion from player repulsion (if any remains after bullet evasion).
                if (fabs(repulsion_x) > 0.01f || fabs(repulsion_y) > 0.01f) {
                    float flee_angle = atan2f(repulsion_y, repulsion_x) * (180.0f / M_PI) + 90.0f;
                    float angle_adjust = flee_angle - enemies[i].angle;
                    while (angle_adjust > 180.0f) angle_adjust -= 360.0f;
                    while (angle_adjust < -180.0f) angle_adjust += 360.0f;
                    DEBUG_PRINT(1, 2, "Evasion (player): flee_angle=%.2f, angle_adjust=%.2f", flee_angle, angle_adjust);
                    if (fabs(angle_adjust) < 5.0f)
                        enemies[i].angle = flee_angle;
                    else if (angle_adjust > 0)
                        enemies[i].angle += 5.0f;
                    else
                        enemies[i].angle -= 5.0f;
                    DEBUG_PRINT(1, 3, "Evasion (enemies): new angle=%.2f", enemies[i].angle);
                    moveStep = 0.8f * difficulty;
                    enemies[i].x -= final_x * moveStep;
                    enemies[i].y -= final_y * moveStep;
                    DEBUG_PRINT(1, 3, "Evasion (enemies): applying reverse thrust");
                    return; // Evasion takes precedence.
                }
                    
                // Determine desired angle from the resulting vector.
                float desiredAngle = atan2f(final_y, final_x) * 180.0f / M_PI;
                float rotationSpeed = 5.0f;
                float angleDifference = desiredAngle - enemies[i].angle;
                while (angleDifference > 180.0f) angleDifference -= 360.0f;
                while (angleDifference < -180.0f) angleDifference += 360.0f;
                if (fabs(angleDifference) < rotationSpeed)
                    enemies[i].angle = desiredAngle;
                else
                    enemies[i].angle += (angleDifference > 0 ? rotationSpeed : -rotationSpeed);
    
                // Move slightly faster than basic enemy.
                moveStep = 0.6f * difficulty;
                enemies[i].x += final_x * moveStep;
                enemies[i].y += final_y * moveStep;
                break;
            }
            case ENEMY_FAST:
            {
                // Fast enemy: similar to basic but with rotation and increased speed.
                float desiredAngle = atan2f(diff_y, diff_x) * 180.0f / M_PI;
                float rotationSpeed = 5.0f;
                float angleDifference = desiredAngle - enemies[i].angle;
                while (angleDifference > 180.0f) angleDifference -= 360.0f;
                while (angleDifference < -180.0f) angleDifference += 360.0f;
                if (fabs(angleDifference) < rotationSpeed)
                    enemies[i].angle = desiredAngle;
                else
                    enemies[i].angle += (angleDifference > 0 ? rotationSpeed : -rotationSpeed);
    
                moveStep = 0.8f * difficulty;
                float rad = enemies[i].angle * (M_PI / 180.0f);
                enemies[i].x += cosf(rad) * moveStep;
                enemies[i].y += sinf(rad) * moveStep;
                break;
            }
            case ENEMY_SPLITTER:
            {
                // Splitting enemy: use basic rotational tracking.
                float desiredAngle = atan2f(diff_y, diff_x) * 180.0f / M_PI;
                float rotationSpeed = 5.0f;
                float angleDifference = desiredAngle - enemies[i].angle;
                while (angleDifference > 180.0f) angleDifference -= 360.0f;
                while (angleDifference < -180.0f) angleDifference += 360.0f;
                if (fabs(angleDifference) < rotationSpeed)
                    enemies[i].angle = desiredAngle;
                else
                    enemies[i].angle += (angleDifference > 0 ? rotationSpeed : -rotationSpeed);
    
                moveStep = 0.5f * difficulty;
                float rad = enemies[i].angle * (M_PI / 180.0f);
                enemies[i].x += cosf(rad) * moveStep;
                enemies[i].y += sinf(rad) * moveStep;
                // Note: When a splitter is hit (in collision detection), spawn two basic enemies.
                break;
            }
            case ENEMY_STEALTH:
            {
                // Toggle visibility based on distance.
                if (distance < 100.0f || distance > 300.0f)
                    enemies[i].visible = 1;
                else
                    enemies[i].visible = 0;
    
                float baseAngle = atan2f(diff_y, diff_x) * 180.0f / M_PI;
                float desiredAngle;
                if (distance >= 100.0f && distance <= 300.0f)
                    desiredAngle = baseAngle + 30.0f;  // offset for sneaking
                else
                    desiredAngle = baseAngle;
    
                // Bullet avoidance.
                float repulsion_x = 0.0f, repulsion_y = 0.0f;
                for (int b = 0; b < pool->count; b++) {
                    if (pool->bullets[b].active && pool->bullets[b].isEnemy == 0) {
                        float bx = pool->bullets[b].x - enemies[i].x;
                        float by = pool->bullets[b].y - enemies[i].y;
                        float bdist = sqrtf(bx * bx + by * by);
                        if (bdist < 50.0f) {
                            repulsion_x -= bx / (bdist + 0.001f);
                            repulsion_y -= by / (bdist + 0.001f);
                        }
                    }
                }
    
                float sneak_x = cosf(desiredAngle * (M_PI / 180.0f));
                float sneak_y = sinf(desiredAngle * (M_PI / 180.0f));
                float final_x = sneak_x + repulsion_x;
                float final_y = sneak_y + repulsion_y;
                float final_norm = sqrtf(final_x * final_x + final_y * final_y);
                if (final_norm > 0) {
                    final_x /= final_norm;
                    final_y /= final_norm;
                }
    
                float targetAngle = atan2f(final_y, final_x) * 180.0f / M_PI;
                float rotationSpeed = 5.0f;
                float angleDifference = targetAngle - enemies[i].angle;
                while (angleDifference > 180.0f) angleDifference -= 360.0f;
                while (angleDifference < -180.0f) angleDifference += 360.0f;
                if (fabs(angleDifference) < rotationSpeed)
                    enemies[i].angle = targetAngle;
                else
                    enemies[i].angle += (angleDifference > 0 ? rotationSpeed : -rotationSpeed);
    
                moveStep = 0.4f * difficulty;
                enemies[i].x += final_x * moveStep;
                enemies[i].y += final_y * moveStep;
                break;
            }
            case ENEMY_BOSS1:
            {
                moveStep = 0.4f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                enemies[i].x += sinf(enemies[i].timer * 0.05f) * 5.0f;
                break;
            }
            case ENEMY_BOSS2:
            {
                if (enemies[i].timer % 240 < 30)
                    moveStep = 1.2f * difficulty;
                else
                    moveStep = 0.4f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            }
            case ENEMY_BOSS3:
            {
                moveStep = 0.5f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep) + ((rand() % 5 - 2) * 0.5f);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep) + ((rand() % 5 - 2) * 0.5f);
                break;
            }
            default:
            {
                moveStep = 0.5f * difficulty;
                if (fabs(diff_x) > 2.0f)
                    enemies[i].x += (diff_x > 0 ? moveStep : -moveStep);
                if (fabs(diff_y) > 2.0f)
                    enemies[i].y += (diff_y > 0 ? moveStep : -moveStep);
                break;
            }
        } // end switch

        DEBUG_PRINT(3, 2, "Updated enemy (type %d) at (%.2f, %.2f), distance=%.2f",
                    enemies[i].type, enemies[i].x, enemies[i].y, distance);
    } // end for

    // Resolve collisions between all active enemies.
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active)
            continue;
        float radius_i = get_collision_radius(enemies[i].type);
        for (int j = i + 1; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active)
                continue;
            float radius_j = get_collision_radius(enemies[j].type);
            float minSeparation = radius_i + radius_j + COLLISION_MARGIN;
            float dx = enemies[j].x - enemies[i].x;
            float dy = enemies[j].y - enemies[i].y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < minSeparation && dist > 0.0f) {
                float overlap = minSeparation - dist;
                float nx = dx / dist;
                float ny = dy / dist;
                enemies[i].x -= nx * (overlap * 0.5f);
                enemies[i].y -= ny * (overlap * 0.5f);
                enemies[j].x += nx * (overlap * 0.5f);
                enemies[j].y += ny * (overlap * 0.5f);
                DEBUG_PRINT(3, 3, "Resolved collision between enemy %d and enemy %d; overlap=%.2f", i, j, overlap);
            }
        }
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
            case ENEMY_SHOOTER: {
                const float halfWidth = 12.0f;
                const float halfHeight = 8.0f;
                float rad = enemies[i].angle * (M_PI / 180.0f);
                float local[4][2] = {
                    { -halfWidth, -halfHeight },
                    {  halfWidth, -halfHeight },
                    {  halfWidth,  halfHeight },
                    { -halfWidth,  halfHeight }
                };
                Sint16 vx[4], vy[4];
                for (int j = 0; j < 4; j++) {
                    float rx = local[j][0] * cos(rad) - local[j][1] * sin(rad);
                    float ry = local[j][0] * sin(rad) + local[j][1] * cos(rad);
                    vx[j] = cx + (int)rx;
                    vy[j] = cy + (int)ry;
                }
                filledPolygonRGBA(renderer, vx, vy, 4, 255, 165, 0, 255);
                break;
            }
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

            switch (enemies[i].type) {
                case ENEMY_BASIC: enemies[i].health = 3; break;
                case ENEMY_SHOOTER: enemies[i].health = 3; enemies[i].shootTimer = 120; break;
                case ENEMY_TANK: enemies[i].health = 10; break;
                case ENEMY_EVASIVE: enemies[i].health = 3; break;
                case ENEMY_FAST: enemies[i].health = 2; break;
                case ENEMY_SPLITTER: enemies[i].health = 3; break;
                case ENEMY_STEALTH: enemies[i].health = 3; break;
                case ENEMY_BOSS1: enemies[i].health = 25; break;
                case ENEMY_BOSS2: enemies[i].health = 50; break;
                case ENEMY_BOSS3: enemies[i].health = 75; break;
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
