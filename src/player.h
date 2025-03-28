#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include "debug.h"

// Ship size restrictions
#define MIN_SHIP_SIZE 10.0f
#define MAX_SHIP_SIZE 40.0f
#define DEFAULT_SHIP_SIZE 20.0f

typedef struct {
    float x, y;
    float vx, vy;
    float angle;   
    int health;
    float energy;
    int shieldActive;
    float size;
} Player;

void init_player(Player *player, int screen_width, int screen_height);
void rotate_player(Player *player, float angle_delta);
void thrust_player(Player *player);
void reverse_thrust(Player *player);
void strafe_left(Player *player);
void strafe_right(Player *player);
void update_player(Player *player);
void update_shield_energy(Player *player);
void activate_shield(Player *player, int active);
void draw_player(Player *player, SDL_Renderer* renderer, int screen_x, int screen_y);
void get_ship_tip(const Player *player, float *tip_x, float *tip_y);
void increase_ship_size(Player *player);
void decrease_ship_size(Player *player);
void reset_ship_size(Player *player);
void wrap_player_position(Player *player);

#endif

