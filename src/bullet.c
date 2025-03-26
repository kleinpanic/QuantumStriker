#include "bullet.h"
#include "debug.h"
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BULLET_SPEED 5.0f
#define INITIAL_BULLET_CAPACITY 100

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
    }
    pool->bullets[oldCount] = b;
    DEBUG_PRINT(3, 2, "Bullet pool expanded from %d to %d; bullet added at index %d", oldCount, pool->count, oldCount);
}

// When shooting, compute the bullet's velocity so that 0° is up.
// We subtract 90° so that the ship's angle (which is defined as the direction the tip is facing)
// is adjusted so that 0° produces upward movement.
void shoot_bullet(BulletPool* pool, float start_x, float start_y, float angle) {
    Bullet b;
    b.active = 1;
    b.x = start_x;
    b.y = start_y;
    float rad = (angle - 90) * (M_PI / 180.0f);
    b.dx = sinf(rad) * BULLET_SPEED;
    b.dy = -cosf(rad) * BULLET_SPEED;
    DEBUG_PRINT(2, 2, "Shooting bullet: start=(%.2f, %.2f), angle=%.2f, velocity=(%.2f, %.2f)",
                start_x, start_y, angle, b.dx, b.dy);
    add_bullet(pool, b);
}

// Update each bullet's position.
void update_bullets(BulletPool* pool) {
    for (int i = 0; i < pool->count; i++) {
        if (pool->bullets[i].active) {
            pool->bullets[i].x += pool->bullets[i].dx;
            pool->bullets[i].y += pool->bullets[i].dy;
            if (pool->bullets[i].x < -5000 || pool->bullets[i].x > 5000 ||
                pool->bullets[i].y < -5000 || pool->bullets[i].y > 5000) {
                pool->bullets[i].active = 0;
                DEBUG_PRINT(3, 2, "Bullet at index %d deactivated (out of bounds)", i);
            }
        }
    }
}

// Draw bullets as small filled rectangles.
void draw_bullets(BulletPool* pool, SDL_Renderer* renderer, float cam_x, float cam_y) {
    SDL_Rect rect;
    rect.w = 4;
    rect.h = 4;
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (int i = 0; i < pool->count; i++) {
        if (pool->bullets[i].active) {
            rect.x = (int)(pool->bullets[i].x - cam_x) - rect.w / 2;
            rect.y = (int)(pool->bullets[i].y - cam_y) - rect.h / 2;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    // Optionally, you can add a debug print here if you want to log how many bullets were drawn.
}

