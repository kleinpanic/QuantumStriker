#include "player.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define THRUST_ACCELERATION 0.5f
#define DAMPING 0.99f
#define MAX_SPEED 15.0f

#define MAX_ENERGY 10.0f
#define DEPLETION_RATE 0.02f
#define REFILL_RATE 0.007f

void init_player(Player *player, int screen_width, int screen_height) {
    (void)screen_width;
    (void)screen_height;
    player->x = 0.0f;
    player->y = 0.0f;
    player->vx = 0.0f;
    player->vy = 0.0f;
    player->angle = 90.0f;  // 90 means up.
    player->health = 5;
    player->energy = MAX_ENERGY;
    player->shieldActive = 0;
}

void rotate_player(Player *player, float angle_delta) {
    player->angle += angle_delta;
    if (player->angle < 0) player->angle += 360;
    if (player->angle >= 360) player->angle -= 360;
}

static void apply_thrust(Player *player, float offset_angle) {
    float rad = (player->angle - 90 + offset_angle) * (M_PI / 180.0f);
    player->vx += sinf(rad) * THRUST_ACCELERATION;
    player->vy -= cosf(rad) * THRUST_ACCELERATION;
}

void thrust_player(Player *player) {
    apply_thrust(player, 0);
}

void reverse_thrust(Player *player) {
    apply_thrust(player, 180);
}

void strafe_left(Player *player) {
    apply_thrust(player, -90);
}

void strafe_right(Player *player) {
    apply_thrust(player, 90);
}

void update_player(Player *player) {
    player->x += player->vx;
    player->y += player->vy;
    float speed = sqrtf(player->vx * player->vx + player->vy * player->vy);
    if (speed > MAX_SPEED) {
        float scale = MAX_SPEED / speed;
        player->vx *= scale;
        player->vy *= scale;
    }
    player->vx *= DAMPING;
    player->vy *= DAMPING;
}

void update_shield_energy(Player *player) {
    if (player->shieldActive) {
        player->energy -= DEPLETION_RATE;
        if (player->energy < 0) {
            player->energy = 0;
            player->shieldActive = 0;
        }
    } else {
        if (player->energy < MAX_ENERGY) {
            player->energy += REFILL_RATE;
            if (player->energy > MAX_ENERGY) player->energy = MAX_ENERGY;
        }
    }
}

void activate_shield(Player *player, int active) {
    if (active && player->energy > 0)
        player->shieldActive = 1;
    else
        player->shieldActive = 0;
}

// Draw the player as an arrow with five points.
// Model coordinates (in the ship's local space):
//   p0 = (0, -size)    -> Tip
//   p1 = (size*0.6, 0)
//   p2 = (size*0.3, size)
//   p3 = (-size*0.3, size)
//   p4 = (-size*0.6, 0)
void draw_player(Player *player, SDL_Renderer* renderer, int screen_x, int screen_y) {
    float size = SHIP_SIZE;
    const int numPoints = 5;
    Sint16 vx[numPoints], vy[numPoints];
    // We want 0° to be up; so adjust by subtracting 90°.
    float angleRad = (player->angle - 90) * (M_PI / 180.0f);
    float model[5][2] = {
        {0, -size},
        {size * 0.6f, 0},
        {size * 0.3f, size},
        {-size * 0.3f, size},
        {-size * 0.6f, 0}
    };
    for (int i = 0; i < numPoints; i++) {
        float x = model[i][0];
        float y = model[i][1];
        float rx = x * cosf(angleRad) - y * sinf(angleRad);
        float ry = x * sinf(angleRad) + y * cosf(angleRad);
        vx[i] = screen_x + (Sint16)rx;
        vy[i] = screen_y + (Sint16)ry;
    }
    filledPolygonRGBA(renderer, vx, vy, numPoints, 0, 255, 0, 255);
    aapolygonRGBA(renderer, vx, vy, numPoints, 0, 180, 0, 255);
    
    if (player->shieldActive) {
        rectangleRGBA(renderer, screen_x - (int)size - 5, screen_y - (int)size - 5,
                      screen_x + (int)size + 5, screen_y + (int)size + 5,
                      0, 200, 255, 255);
    }
}

// Compute the tip of the ship (spawn point for bullets).
// In model space, the tip is at (0, -size). Rotate it and add player's world position.
void get_ship_tip(const Player *player, float *tip_x, float *tip_y) {
    float size = SHIP_SIZE;
    float angleRad = (player->angle - 90) * (M_PI / 180.0f);
    float tx = size * sinf(angleRad);
    float ty = -size * cosf(angleRad);
    *tip_x = player->x + tx;
    *tip_y = player->y + ty;
}
