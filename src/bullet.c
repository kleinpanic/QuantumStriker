#include "bullet.h"
#include "config.h"
#include "debug.h"
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Initialize a dynamic bullet pool.
void init_bullet_pool(BulletPool* pool) {
    pool->count = INITIAL_BULLET_CAPACITY;
    pool->bullets = (Bullet*)malloc(pool->count * sizeof(Bullet));
    if (!pool->bullets) {
        DEBUG_PRINT(2, 0, "Failed to allocate bullet pool of size %d", pool->count);
        return;
    }
    for (int i = 0; i < pool->count; i++) {
        pool->bullets[i].active = 0;
        pool->bullets[i].isEnemy = 0;
        pool->bullets[i].damage = 1; // default damage 
        pool->bullets[i].spawn_x = 0.0f;
        pool->bullets[i].spawn_y = 0.0f;
    }
    DEBUG_PRINT(2, 3, "Bullet pool initialized with capacity %d", pool->count);
}

// Free the bullet pool.
void free_bullet_pool(BulletPool* pool) {
    free(pool->bullets);
    pool->bullets = NULL;
    pool->count = 0;
    DEBUG_PRINT(2, 3, "Bullet pool freed");
}

// Internal: Add a new bullet into the pool, expanding if needed.
static void add_bullet(BulletPool* pool, Bullet b) {
    for (int i = 0; i < pool->count; i++) {
        if (!pool->bullets[i].active) {
            pool->bullets[i] = b;
            DEBUG_PRINT(3, 2, "Bullet added at index %d", i);
            return;
        }
    }
    int oldCount = pool->count;
    pool->count *= 2;
    pool->bullets = (Bullet*)realloc(pool->bullets, pool->count * sizeof(Bullet));
    if (!pool->bullets) {
        DEBUG_PRINT(2, 0, "Failed to reallocate bullet pool to size %d", pool->count);
        return;
    }
    for (int i = oldCount; i < pool->count; i++) {
        pool->bullets[i].active = 0;
        pool->bullets[i].isEnemy = 0;
        pool->bullets[i].damage = 1;
        pool->bullets[i].spawn_x = 0.0f;
        pool->bullets[i].spawn_y = 0.0f;
    }
    pool->bullets[oldCount] = b;
    DEBUG_PRINT(3, 2, "Bullet pool expanded from %d to %d; bullet added at index %d", oldCount, pool->count, oldCount);
}

// When shooting, compute bullet velocity so that 0° is up. Also store spawn position.
void shoot_bullet(BulletPool* pool, float start_x, float start_y, float angle, int isEnemy) {
    Bullet b;
    b.active = 1;
    b.x = start_x;
    b.y = start_y;
    b.spawn_x = start_x;
    b.spawn_y = start_y;
    float rad = (angle - 90) * (M_PI / 180.0f);
    if (g_dev_auto_mode) {
        float dev_bullet_speed = BULLET_SPEED * AI_BULLET_SPEED_MULTIPLIER;
        b.dx = sinf(rad) * dev_bullet_speed;
        b.dy = -cosf(rad) * dev_bullet_speed;
        b.damage = AI_DEFAULT_BULLET_DAMAGE;
    } else {
        b.dx = sinf(rad) * BULLET_SPEED;
        b.dy = -cosf(rad) * BULLET_SPEED;
        b.damage = 1;
    }
    b.isEnemy = isEnemy;
    DEBUG_PRINT(2, 2, "Shooting bullet: start=(%.2f, %.2f), angle=%.2f, velocity=(%.2f, %.2f), isEnemy=%d, damage=%d",
                start_x, start_y, angle, b.dx, b.dy, isEnemy, b.damage);
    add_bullet(pool, b);
}

// Update each bullet's position and despawn if too far from its spawn.
void update_bullets(BulletPool* pool) {
    for (int i = 0; i < pool->count; i++) {
        if (pool->bullets[i].active) {
            pool->bullets[i].x += pool->bullets[i].dx;
            pool->bullets[i].y += pool->bullets[i].dy;
            // Dynamic despawn: if the bullet has traveled farther than BULLET_DESPAWN_DISTANCE, deactivate it.
            float dx = pool->bullets[i].x - pool->bullets[i].spawn_x;
            float dy = pool->bullets[i].y - pool->bullets[i].spawn_y;
            float travel = sqrtf(dx*dx + dy*dy);
            if (travel > BULLET_DESPAWN_DISTANCE) {
                pool->bullets[i].active = 0;
                DEBUG_PRINT(3, 2, "Bullet at index %d deactivated (travel distance %.2f > %.2f)", i, travel, BULLET_DESPAWN_DISTANCE);
            }
        }
    }
}

// Draw bullets as small filled rectangles.
void draw_bullets(BulletPool* pool, SDL_Renderer* renderer, float cam_x, float cam_y) {
    SDL_Rect rect;
    rect.w = 4;
    rect.h = 4;
    for (int i = 0; i < pool->count; i++) {
        if (pool->bullets[i].active) {
            rect.x = (int)(pool->bullets[i].x - cam_x) - rect.w / 2;
            rect.y = (int)(pool->bullets[i].y - cam_y) - rect.h / 2;
            if (pool->bullets[i].isEnemy)
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            else
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

