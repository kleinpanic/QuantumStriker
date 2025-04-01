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

void split_enemy(Enemy enemies[], int index) {
    // Only proceed if the enemy is indeed a splitter.
    if (enemies[index].type != ENEMY_SPLITTER)
        return;

    // Use the splitter enemy's current angle as the base.
    float baseAngle = enemies[index].angle;
    float angle1 = baseAngle + 30.0f;
    float angle2 = baseAngle - 30.0f;

    // For each of the two new enemies:
    for (int i = 0; i < 2; i++) {
        int slot = -1;
        // Find an inactive enemy slot.
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) {
                slot = j;
                break;
            }
        }
        if (slot == -1) {
            DEBUG_PRINT(3, 2, "No free slot to spawn split enemy.");
            break;
        }
        // Choose the angle for this new enemy.
        float spawnAngle = (i == 0) ? angle1 : angle2;
        // Generate a random offset distance between 10 and 15 units.
        float offsetDistance = (25.0f + (rand() % 26)); // should return 10-20
        // Calculate the spawn position offset from the splitter enemy's position.
        enemies[slot].x = enemies[index].x + offsetDistance * cosf(spawnAngle * (M_PI / 180.0f));
        enemies[slot].y = enemies[index].y + offsetDistance * sinf(spawnAngle * (M_PI / 180.0f));
        enemies[slot].angle = spawnAngle;  // Face in the direction of the spawn.
        enemies[slot].type = ENEMY_BASIC;  // Or change to a different type if desired.
        enemies[slot].health = 3;          // Set health for a basic enemy.
        enemies[slot].active = 1;
        enemies[slot].timer = 0;
        enemies[slot].visible = 1;
        DEBUG_PRINT(3, 2, "Split enemy spawned in slot %d at (%.2f, %.2f)",
                    slot, enemies[slot].x, enemies[slot].y);
    }
}

static void rotate_toward(float *current, float target, float maxDelta) {
    float diff = target - *current;
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;
    if (fabs(diff) <= maxDelta) {
        *current = target;
    } else {
        *current += (diff > 0 ? maxDelta : -maxDelta);
    }
    // Ensure the angle stays in [0,360)
    while (*current < 0) *current += 360.0f;
    while (*current >= 360.0f) *current -= 360.0f;
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
                float rotationSpeed = 5.0f;  // Slower rotation than shooter.
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
                DEBUG_PRINT(3, 2, "Spawned Evasive");
                // Parameters for behavior
                const float attractionWeight = 2.0f;
                const float repulsionWeight = 5.0f;
                const float bulletDangerDistance = 60.0f;
                const float secondaryBulletDangerDistance = 120.0f;
                const float maxRotDelta = 7.0f; // Maximum Rotation per frame in degrees.
                const float moveSpeed = 0.8f * difficulty;
                // Base attraction toward the player.
                float norm = (distance > 0) ? distance : 1.0f;
                float att_x = (diff_x / norm) * attractionWeight;
                float att_y = (diff_y / norm) * attractionWeight;
    
                // Bullet avoidance: scan for nearby player bullets.
                float rep_x = 0.0f, rep_y = 0.0f;
                int critical = 0;
                for (int j = 0; j < pool->count; j++) {
                    if (pool->bullets[j].active && pool->bullets[j].isEnemy == 0) {
                        float bx = pool->bullets[j].x - enemies[i].x;
                        float by = pool->bullets[j].y - enemies[i].y;
                        float bdist = sqrtf(bx * bx + by * by);
                        DEBUG_PRINT(1, 2, "player Bullet %d: bdist=%.2f", j, bdist);
                        if (bdist < bulletDangerDistance) {
                            critical = 1;
                            float force = repulsionWeight * (bulletDangerDistance - bdist) / bulletDangerDistance;
                            rep_x -= (bx / bdist) * force;
                            rep_y -= (by / bdist) * force;
                        } else if (bdist < secondaryBulletDangerDistance) {
                            float force = (repulsionWeight / 2.0f) * (secondaryBulletDangerDistance - bdist) / secondaryBulletDangerDistance;
                            rep_x -= (bx / bdist) * force;
                            rep_y -= (by / bdist) * force;
                            DEBUG_PRINT(1, 2, "Player Bullet %d: Repulsive added (%.2f, %.2f)", i, rep_x, rep_y);
                        }
                    }
                }

                float final_x, final_y;
                if (critical) {
                    final_x = rep_x;
                    final_y = rep_y;
                    float randomOffset = ((rand() % 200) - 100) / 100.0f; // In range -1 to 1
                    final_x += randomOffset;
                    final_y += randomOffset;
                } else {
                    final_x = att_x + rep_x;
                    final_y = att_y + rep_y;
                }


                float final_norm = sqrtf(final_x * final_x + final_y * final_y);
                if (final_norm > 0) {
                    final_x /= final_norm;
                    final_y /= final_norm;
                }

                // Determine desired angle from the resulting vector.
                float desiredAngle = atan2f(final_y, final_x) * 180.0f / M_PI;
                rotate_toward(&enemies[i].angle, desiredAngle, maxRotDelta);
                // Move slightly faster than basic enemy.
                float rad = enemies[i].angle * (M_PI / 180.0f);
                enemies[i].x += cosf(rad) * moveSpeed;
                enemies[i].y += sinf(rad) * moveSpeed;
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

static void filledRotatedEllipse(SDL_Renderer *renderer, int cx, int cy, int rx, int ry, float angle, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    const int numSegments = 20; // Increase for a smoother ellipse.
    Sint16 vx[numSegments], vy[numSegments];
    float radAngle = angle * (M_PI / 180.0f);
    for (int i = 0; i < numSegments; i++) {
        // Get the vertex on the ellipse in local space.
        float theta = (2.0f * M_PI * i) / numSegments;
        float localX = rx * cosf(theta);
        float localY = ry * sinf(theta);
        // Rotate the vertex by enemy's angle.
        float rotatedX = localX * cosf(radAngle) - localY * sinf(radAngle);
        float rotatedY = localX * sinf(radAngle) + localY * cosf(radAngle);
        vx[i] = cx + (int)rotatedX;
        vy[i] = cy + (int)rotatedY;
    }
    filledPolygonRGBA(renderer, vx, vy, numSegments, r, g, b, a);
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
                filledRotatedEllipse(renderer, cx, cy, 15, 10, enemies[i].angle, 255, 0, 0, 255);
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
                filledRotatedEllipse(renderer, cx, cy, 20, 14, enemies[i].angle, 0, 0, 255, 255);
                break;
            case ENEMY_EVASIVE: {
                const float halfWidth = 12.0f;
                const float halfHeight = 8.0f;
                float rad = enemies[i].angle * (M_PI / 180.0f);
                // Define triangle in local space.
                float local[3][2] = {
                    { 0, -halfHeight },       // top
                    { halfWidth, halfHeight },  // bottom right
                    { -halfWidth, halfHeight }  // bottom left
                };
                Sint16 vx[3], vy[3];
                for (int j = 0; j < 3; j++) {
                    float rx = local[j][0] * cos(rad) - local[j][1] * sin(rad);
                    float ry = local[j][0] * sin(rad) + local[j][1] * cos(rad);
                    vx[j] = cx + (int)rx;
                    vy[j] = cy + (int)ry;
                }
                filledPolygonRGBA(renderer, vx, vy, 3, 255, 165, 0, 255);
                break;
            }
            case ENEMY_FAST:
                filledRotatedEllipse(renderer, cx, cy, 10, 7, enemies[i].angle, 255, 255, 0, 255);
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
            if (g_forced_enemy_type != -1) {
                enemies[i].type = g_forced_enemy_type;
            } else {
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
