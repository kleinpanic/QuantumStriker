#ifndef BULLET_H
#define BULLET_H

#include <SDL2/SDL.h>
#include "debug.h"

typedef struct {
    float x, y;
    float dx, dy;
    int active;
    int isEnemy;  // 0 = player's bullet, 1 = enemy bullet
    int damage;
    float spawn_x, spawn_y;
} Bullet;

typedef struct {
    Bullet* bullets;
    int count;      // capacity of the bullet pool
} BulletPool;

void init_bullet_pool(BulletPool* pool);
void free_bullet_pool(BulletPool* pool);
void update_bullets(BulletPool* pool);
void draw_bullets(BulletPool* pool, SDL_Renderer* renderer, float cam_x, float cam_y);
void shoot_bullet(BulletPool* pool, float start_x, float start_y, float angle, int isEnemy);

#endif

