#ifndef CONFIG_H
#define CONFIG_H

#include <signal.h>  // Ensure sig_atomic_t is defined

extern int g_fullscreen;   // 0 = resizable, 1 = full screen
extern int g_dev_auto_mode;
extern int g_testing_mode;
extern int g_forced_enemy_type;
extern int shakeTimer;
extern float shakeMagnitude;
extern volatile sig_atomic_t g_exit_requested;

/* Background configuratiosn */
#define ENABLE_GRID 0  // 0: grid off, 1: grid on
#ifndef WORLD_BORDER
#define WORLD_BORDER 20000   // Defines a 10000 x 10000 world
#endif
#define GALACTIC_OBJECT_DENSITY 0.000005  // objects per pixel²
#define GALACTIC_OBJECT_DENSITY_NUMERATOR 5
#define GALACTIC_OBJECT_DENSITY_DENOMINATOR 1000000
#define NUM_BG_OBJECTS (((WORLD_BORDER) * (WORLD_BORDER) * (GALACTIC_OBJECT_DENSITY_NUMERATOR)) / (GALACTIC_OBJECT_DENSITY_DENOMINATOR))

/* AI Configurations for development auto mode */
#define AI_DEFAULT_HEALTH 9999 // Set to -1 for infinite health (where it auto refills)
#define AI_USE_COLLISIONS 1 // 1: collisions decrease AI's health. 0: Collisions don't decrease AI's health
#define AI_DEFAULT_ENERGY 9999 // Current implementation does now use sheild. Remove usage. 
#define AI_DEFAULT_BULLET_DAMAGE 5 // default AI bullet damage. Player damage is 1. 
#define AI_PROGRESS_MULTIPLIER 2.0f
#define AI_REFILL_RATE 10.0f
#define AI_MIN_ENERGY 10 // Value that if reached, energy will reset
#define AI_PIERCING_SHOT 1 //1: Bullet pierce enemies; 0: normal behavior.
#define AI_BULLET_SPEED_MULTIPLIER 2.0f
#define AI_OFFENSIVE_ROTATION_SPEED 5.0f
#define AI_EVASION_ROTATION_SPEED 2.0f

/* Bullet Configurations */
#define BULLET_DESPAWN_DISTANCE 1000.0f 
#define BULLET_SPEED 5.0f
#define INITIAL_BULLET_CAPACITY 100

/* Player Configurations */
#define THRUST_ACCELERATION 0.5f
#define DAMPING 0.99f
#define MAX_SPEED 10.0f
#define MAX_ENERGY 10.0f
#define DEPLETION_RATE 0.02f
#define REFILL_RATE 0.007f

/* Enemy Configurations */
#define MAX_ENEMIES 50
#define COLLISION_MARGIN 5.0f

/* Highscores flag configuration */
#define HIGHSCORE_FLAG_MAX_ENTRY_NUMBER 10
#define MAX_BLOCKS 1000  // Maximum number of blocks to read from the blockchain file

/* Random */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif /* CONFIG_H */

