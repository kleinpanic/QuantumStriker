#ifndef ENEMY_H
#define ENEMY_H

#include "bullet.h"  // Needed for the BulletPool type.
#include <SDL2/SDL.h>
#include "debug.h"
#include "config.h"

// Define 10 enemy types (7 regular + 3 bosses)
typedef enum {
    ENEMY_BASIC = 0,
    ENEMY_SHOOTER,
    ENEMY_TANK,
    ENEMY_EVASIVE,
    ENEMY_FAST,
    ENEMY_SPLITTER,
    ENEMY_STEALTH,
    ENEMY_BOSS1,
    ENEMY_BOSS2,
    ENEMY_BOSS3
} EnemyType;

typedef struct {
    float x, y;
    int health;
    EnemyType type;   // enemy type
    int active;
    int timer;       // general-purpose timer for AI state changes
    int shootTimer;  // for shooter enemy (frames until next shot)
    int visible;     // for stealth enemy (1: visible, 0: invisible)
    float angle; // field for the enemy to rotate
} Enemy;

// Initializes the enemy array.
void init_enemies(Enemy enemies[]);

// Updates enemy behavior based on player position and difficulty.
void update_enemies(Enemy enemies[], float player_x, float player_y, float difficulty, BulletPool* pool);

// Draws enemies with different visual styles based on their type.
void draw_enemies(Enemy enemies[], SDL_Renderer* renderer, float cam_x, float cam_y);

// Spawns an enemy based on the current score.
void spawn_enemy(Enemy enemies[], float player_x, float player_y, int score);

// Declaration for enemy_shoot, which fires a bullet from a shooter enemy toward the player.
void enemy_shoot(Enemy *enemy, BulletPool *pool, float player_x, float player_y);

void split_enemy(Enemy enemies[], int index); 
#endif

