#include "player.h"
#include "debug.h"
#include "config.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>

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
    player->size = DEFAULT_SHIP_SIZE;
    DEBUG_PRINT(2, 3, "Player initialized: pos=(%.2f, %.2f), angle=%.2f, health=%d, energy=%.2f, size=%.2f",
                player->x, player->y, player->angle, player->health, player->energy, player->size);
}

void wrap_player_position(Player *player) {
    float halfBorder = WORLD_BORDER / 2.0f;
    if (player->x > halfBorder)
        player->x = -halfBorder;
    else if (player->x < -halfBorder)
        player->x = halfBorder;
    
    if (player->y > halfBorder)
        player->y = -halfBorder;
    else if (player->y < -halfBorder)
        player->y = halfBorder;
}

void rotate_player(Player *player, float angle_delta) {
    player->angle += angle_delta;
    if (player->angle < 0) player->angle += 360;
    if (player->angle >= 360) player->angle -= 360;
    DEBUG_PRINT(3, 2, "Player rotated: new angle=%.2f", player->angle);
}

static void apply_thrust(Player *player, float offset_angle) {
    float rad = (player->angle - 90 + offset_angle) * (M_PI / 180.0f);
    player->vx += sinf(rad) * THRUST_ACCELERATION;
    player->vy -= cosf(rad) * THRUST_ACCELERATION;
    DEBUG_PRINT(3, 2, "Applied thrust: offset=%.2f, new velocity=(%.2f, %.2f)",
                offset_angle, player->vx, player->vy);
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
        DEBUG_PRINT(3, 2, "Speed capped: new velocity=(%.2f, %.2f)", player->vx, player->vy);
    }
    player->vx *= DAMPING;
    player->vy *= DAMPING;
    DEBUG_PRINT(3, 2, "Player updated: pos=(%.2f, %.2f), velocity=(%.2f, %.2f)",
                player->x, player->y, player->vx, player->vy);
}

void update_shield_energy(Player *player) {
    if (player->shieldActive) {
        player->energy -= DEPLETION_RATE;
        if (player->energy < 0) {
            player->energy = 0;
            player->shieldActive = 0;
            DEBUG_PRINT(3, 0, "Shield deactivated due to energy depletion");
        }
    } else {
        float refill = REFILL_RATE;
        float max_energy = MAX_ENERGY;
        if (g_dev_auto_mode) {
            refill = AI_REFILL_RATE;
            max_energy = AI_DEFAULT_ENERGY;
        }
        if (player->energy < max_energy) {
            player->energy += refill;
            if (player->energy > max_energy) player->energy = max_energy;
            if (g_dev_auto_mode) {
                if (player->energy < AI_MIN_ENERGY) player->energy = max_energy; 
            }
        }
    }
    DEBUG_PRINT(3, 2, "Shield energy updated: energy=%.2f, shieldActive=%d",
                player->energy, player->shieldActive);
}

void activate_shield(Player *player, int active) {
    if (active && player->energy > 0)
        player->shieldActive = 1;
    else
        player->shieldActive = 0;
    DEBUG_PRINT(3, 2, "Shield activation set to %d", player->shieldActive);
}

// Draw the player as an arrow with five points.
// Model coordinates (in the ship's local space):
//   p0 = (0, -size)    -> Tip
//   p1 = (size*0.6, 0)
//   p2 = (size*0.3, size)
//   p3 = (-size*0.3, size)
//   p4 = (-size*0.6, 0)
void draw_player(Player *player, SDL_Renderer* renderer, int screen_x, int screen_y) {
    //float size = SHIP_SIZE;
    const int numPoints = 5;
    Sint16 vx[numPoints], vy[numPoints];
    // We want 0° to be up; so adjust by subtracting 90°.
    float angleRad = (player->angle - 90) * (M_PI / 180.0f);
    float model[5][2] = {
        {0, -player->size},
        {player->size * 0.6f, 0},
        {player->size * 0.3f, player->size},
        {-player->size * 0.3f, player->size},
        {-player->size * 0.6f, 0}
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
        rectangleRGBA(renderer, screen_x - (int)player->size - 5, screen_y - (int)player->size - 5,
                      screen_x + (int)player->size + 5, screen_y + (int)player->size + 5,
                      0, 200, 255, 255);
    }
    DEBUG_PRINT(3, 2, "Player drawn at screen position (%d, %d)", screen_x, screen_y);
}

// Compute the tip of the ship (spawn point for bullets).
// In model space, the tip is at (0, -size). Rotate it and add player's world position.
void get_ship_tip(const Player *player, float *tip_x, float *tip_y) {
    float angleRad = (player->angle - 90) * (M_PI / 180.0f);
    float tx = player->size * sinf(angleRad);
    float ty = -player->size * cosf(angleRad);
    *tip_x = player->x + tx;
    *tip_y = player->y + ty;
    DEBUG_PRINT(3, 2, "Ship tip computed: (%.2f, %.2f)", *tip_x, *tip_y);
}

void increase_ship_size(Player *player) {
    if (player->size < MAX_SHIP_SIZE) {
        player->size += 1.0f; // Increase size by 1 unit
        if (player->size > MAX_SHIP_SIZE)
            player->size = MAX_SHIP_SIZE;
        DEBUG_PRINT(3, 2, "Increased ship size to %.2f", player->size);
    }
}

void decrease_ship_size(Player *player) {
    if (player->size > MIN_SHIP_SIZE) {
        player->size -= 1.0f; // Decrease size by 1 unit
        if (player->size < MIN_SHIP_SIZE)
            player->size = MIN_SHIP_SIZE;
        DEBUG_PRINT(3, 2, "Decreased ship size to %.2f", player->size);
    }
}

void reset_ship_size(Player *player) {
    player->size = DEFAULT_SHIP_SIZE;
    DEBUG_PRINT(3, 2, "Reset ship size to default (%.2f)", player->size);
}
