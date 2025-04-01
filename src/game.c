#include "game.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "background.h"
#include "score.h"
#include "blockchain.h"
#include "signature.h"
#include "encryption.h"
#include "debug.h"
#include "config.h"
#include "menus.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define FRAME_DELAY 15   // milliseconds per frame
#define BLOCKCHAIN_FILE "highscore/blockchain.txt"
#define DIFFICULTY 4     // PoW difficulty: number of leading zeros required

int shakeTimer = 0;
float shakeMagnitude = 0.0f;

// Helper function to render text using SDL_ttf.
void render_text(SDL_Renderer* renderer, TTF_Font* font, int x, int y, const char* text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface)
        return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

/*
 * dev_ai_control controls the player AI behavior in auto dev mode.
 *
 * Operation Overview:
 *
 * 1. Visibility & Target Selection:
 *    - The camera origin is computed from (player->x - screen_width/2, player->y - screen_height/2).
 *    - For each active enemy, its screen position is computed.
 *      Enemies outside a 50-pixel margin are skipped.
 *    - The closest visible enemy is selected as the target.
 *
 * 2. Enemy Bullet Detection:
 *    - The bullet pool is scanned for active enemy bullets.
 *      Any enemy bullet within BULLET_DANGER_DISTANCE (150 units) adds a repulsion force (scaled by BULLET_REPULSION_FACTOR)
 *      and sets a flag to force bullet-evading behavior.
 *
 * 3. Shield Activation & Evasion:
 *    - The shield is activated if the target is very close (less than SHIELD_DISTANCE) or if an enemy bullet is dangerously close.
 *    - If enemy bullets are dangerously close (enemyBulletTooClose is true), the AI forces the shield and immediately
 *      rotates away from the bullet threat using the computed repulsion vector, applies reverse thrust, and returns.
 *
 * 4. Offensive Engagement:
 *    - If no dangerous bullet is present and a target exists, the desired angle is computed using:
 *         desired_angle = atan2(target->y - player->y, target->x - player->x) * (180/π) + 180
 *      so that when drawn (using (player->angle - 90)) the ship’s tip (model point (0, -size)) points toward the target.
 *    - The AI rotates toward that angle at OFFENSIVE_ROTATION_SPEED.
 *    - If the target is far (distance > SHOOTING_RANGE) and nearly aligned (angle difference < 10°), thrust is applied.
 *      If within range, the ship brakes to stabilize before shooting.
 *    - If aligned within 5° and within range, a bullet is fired from the ship’s tip.
 *
 * Note: CENTER_BONUS is currently logged for potential future use.
 */

void dev_ai_control(Player *player, Enemy enemies[], BulletPool *bulletPool, int screen_width, int screen_height) {

    // AI parameters.
    const float SHOOTING_RANGE            = 300.0f;
    const float DANGER_DISTANCE           = 80.0f;
    const float SHIELD_DISTANCE           = 50.0f;
    const float BULLET_DANGER_DISTANCE    = 150.0f;  // Increased danger distance for bullets.
    const float BULLET_REPULSION_FACTOR   = 3.0f;    // Scale factor for bullet repulsion.

    float best_distance = 1e9;
    Enemy *target = NULL;
    float repulsion_x = 0.0f, repulsion_y = 0.0f;
    int enemyBulletTooClose = 0;  // Flag to indicate dangerous enemy bullets.

    // Calculate camera origin.
    float cam_x = player->x - screen_width / 2.0f;
    float cam_y = player->y - screen_height / 2.0f;

    // Evaluate all active enemies.
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active)
            continue;

        // Skip stealth enemies that are invisible
        if (enemies[i].type == ENEMY_STEALTH && !enemies[i].visible)
            continue;

        float dx = enemies[i].x - player->x;
        float dy = enemies[i].y - player->y;
        float distance = sqrtf(dx * dx + dy * dy);

        // Compute enemy's screen position.
        float enemy_screen_x = enemies[i].x - cam_x;
        float enemy_screen_y = enemies[i].y - cam_y;
        if (enemy_screen_x < -50 || enemy_screen_x > screen_width + 50 ||
            enemy_screen_y < -50 || enemy_screen_y > screen_height + 50) {
            DEBUG_PRINT(1, 2, "Enemy %d: offscreen (screen pos: %.2f, %.2f), skipped", i, enemy_screen_x, enemy_screen_y);
            continue;
        }
        float center_dx = enemy_screen_x - screen_width / 2.0f;
        float center_dy = enemy_screen_y - screen_height / 2.0f;
        float screen_dist = sqrtf(center_dx * center_dx + center_dy * center_dy);
        DEBUG_PRINT(1, 2, "Enemy %d: distance=%.2f, screen pos=(%.2f, %.2f), center_dist=%.2f", i, distance, enemy_screen_x, enemy_screen_y, screen_dist);

        if (distance < best_distance) {
            best_distance = distance;
            target = &enemies[i];
            DEBUG_PRINT(1, 2, "New target selected: enemy %d, distance=%.2f", i, distance);
        }
        if (distance < DANGER_DISTANCE) {
            repulsion_x -= (dx / distance) * (DANGER_DISTANCE - distance);
            repulsion_y -= (dy / distance) * (DANGER_DISTANCE - distance);
            DEBUG_PRINT(1, 2, "Enemy %d: repulsion added (%.2f, %.2f)", i, repulsion_x, repulsion_y);
        }
    }

    // Evaluate enemy bullets.
    for (int i = 0; i < bulletPool->count; i++) {
        if (bulletPool->bullets[i].active && bulletPool->bullets[i].isEnemy == 1) {
            float bx = bulletPool->bullets[i].x - player->x;
            float by = bulletPool->bullets[i].y - player->y;
            float bdist = sqrtf(bx * bx + by * by);
            DEBUG_PRINT(1, 2, "Enemy bullet %d: bdist=%.2f", i, bdist);
            if (bdist < BULLET_DANGER_DISTANCE) {
                enemyBulletTooClose = 1;
                repulsion_x -= (bx / bdist) * (BULLET_DANGER_DISTANCE - bdist) * BULLET_REPULSION_FACTOR;
                repulsion_y -= (by / bdist) * (BULLET_DANGER_DISTANCE - bdist) * BULLET_REPULSION_FACTOR;
                DEBUG_PRINT(1, 2, "Enemy bullet %d: repulsion added (%.2f, %.2f)", i, repulsion_x, repulsion_y);
            }
        }
    }
    if (enemyBulletTooClose) {
        DEBUG_PRINT(1, 3, "Enemy bullets are dangerously close; forcing shield activation and bullet evasion.");
        activate_shield(player, 1);
        // Evasion for bullets takes precedence: rotate away from bullet threat.
        float flee_angle = atan2f(repulsion_y, repulsion_x) * (180.0f / M_PI) + 90.0f;
        float angle_adjust = flee_angle - player->angle;
        while (angle_adjust > 180.0f) angle_adjust -= 360.0f;
        while (angle_adjust < -180.0f) angle_adjust += 360.0f;
        DEBUG_PRINT(1, 2, "Bullet Evasion: flee_angle=%.2f, angle_adjust=%.2f", flee_angle, angle_adjust);
        if (fabs(angle_adjust) < AI_EVASION_ROTATION_SPEED)
            player->angle = flee_angle;
        else if (angle_adjust > 0)
            player->angle += AI_EVASION_ROTATION_SPEED;
        else
            player->angle -= AI_EVASION_ROTATION_SPEED;
        DEBUG_PRINT(1, 3, "Bullet Evasion: new angle=%.2f", player->angle);
        reverse_thrust(player);
        DEBUG_PRINT(1, 3, "Bullet Evasion: applying reverse thrust");
        return; // Do not continue offensive behavior while bullets are too close.
    }

    // Shield activation based on enemy target (if no dangerous bullets).
    if (target != NULL) {
        float dx = target->x - player->x;
        float dy = target->y - player->y;
        float dist = sqrtf(dx * dx + dy * dy);
        DEBUG_PRINT(1, 2, "Target distance for shield check: %.2f", dist);
        if (dist < SHIELD_DISTANCE) {
            activate_shield(player, 1);
            DEBUG_PRINT(1, 3, "Shield activated (target distance %.2f < %.2f)", dist, SHIELD_DISTANCE);
            float tip_x, tip_y;
            get_ship_tip(player, &tip_x, &tip_y);
            shoot_bullet(bulletPool, tip_x, tip_y, player->angle, 0);
        } else {
            activate_shield(player, 0);
            DEBUG_PRINT(1, 3, "Shield deactivated (target distance %.2f >= %.2f)", dist, SHIELD_DISTANCE);
        }
    } else {
        activate_shield(player, 0);
        DEBUG_PRINT(1, 3, "No target found; shield deactivated");
    }
    
    // Evasion from enemy repulsion (if any remains after bullet evasion).
    if (fabs(repulsion_x) > 0.01f || fabs(repulsion_y) > 0.01f) {
        float flee_angle = atan2f(repulsion_y, repulsion_x) * (180.0f / M_PI) + 90.0f;
        float angle_adjust = flee_angle - player->angle;
        while (angle_adjust > 180.0f) angle_adjust -= 360.0f;
        while (angle_adjust < -180.0f) angle_adjust += 360.0f;
        DEBUG_PRINT(1, 2, "Evasion (enemies): flee_angle=%.2f, angle_adjust=%.2f", flee_angle, angle_adjust);
        if (fabs(angle_adjust) < AI_EVASION_ROTATION_SPEED)
            player->angle = flee_angle;
        else if (angle_adjust > 0)
            player->angle += AI_EVASION_ROTATION_SPEED;
        else
            player->angle -= AI_EVASION_ROTATION_SPEED;
        DEBUG_PRINT(1, 3, "Evasion (enemies): new angle=%.2f", player->angle);
        reverse_thrust(player);
        DEBUG_PRINT(1, 3, "Evasion (enemies): applying reverse thrust");
        return; // Evasion takes precedence.
    }
    
    // Offensive engagement.
    if (target != NULL) {
        float desired_angle = atan2f(target->y - player->y, target->x - player->x) * (180.0f / M_PI) + 180.0f;
        float angle_adjust = desired_angle - player->angle;
        while (angle_adjust > 180.0f) angle_adjust -= 360.0f;
        while (angle_adjust < -180.0f) angle_adjust += 360.0f;
        DEBUG_PRINT(1, 2, "Offense: desired_angle=%.2f, angle_adjust=%.2f", desired_angle, angle_adjust);
        if (fabs(angle_adjust) < AI_OFFENSIVE_ROTATION_SPEED)
            player->angle = desired_angle;
        else if (angle_adjust > 0)
            player->angle += AI_OFFENSIVE_ROTATION_SPEED;
        else
            player->angle -= AI_OFFENSIVE_ROTATION_SPEED;
        DEBUG_PRINT(1, 3, "Offense: new angle=%.2f", player->angle);
    
        float dx = target->x - player->x;
        float dy = target->y - player->y;
        float distance = sqrtf(dx * dx + dy * dy);
        DEBUG_PRINT(1, 2, "Offense: distance to target = %.2f", distance);
        
        if (distance > SHOOTING_RANGE) {
            if (fabs(angle_adjust) < 10.0f) {
                thrust_player(player);
                DEBUG_PRINT(1, 3, "Offense: enemy far and aligned, applying thrust to approach");
            } else {
                DEBUG_PRINT(1, 3, "Offense: enemy far but not aligned (angle_adjust=%.2f), no thrust", angle_adjust);
            }
        } else {
            player->vx *= 0.8f;
            player->vy *= 0.8f;
            DEBUG_PRINT(1, 3, "Offense: enemy within shooting range, braking to stabilize");
            float tip_x, tip_y;
            get_ship_tip(player, &tip_x, &tip_y);
            shoot_bullet(bulletPool, tip_x, tip_y, player->angle, 0);
        }
    
        if (fabs(angle_adjust) < 5.0f && distance <= SHOOTING_RANGE) {
            float tip_x, tip_y;
            get_ship_tip(player, &tip_x, &tip_y);
            shoot_bullet(bulletPool, tip_x, tip_y, player->angle, 0);
            DEBUG_PRINT(1, 3, "Offense: aligned (angle_adjust=%.2f) and within range, shooting", angle_adjust);
        } else {
            DEBUG_PRINT(1, 2, "Offense: not shooting (angle_adjust=%.2f, distance=%.2f)", angle_adjust, distance);
        }
    } else {
        DEBUG_PRINT(1, 3, "No target detected: remaining stationary");
    }
}

// Prompt the user for a username via the SDL window.
// If Esc is pressed, the input immediately becomes "default".
char* prompt_username(SDL_Renderer* renderer, TTF_Font* font, int screen_width, int screen_height) {
    char input[100] = {0};
    int input_length = 0;
    SDL_StartTextInput();
    SDL_Event event;
    int done = 0;
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
                break;
            } else if (event.type == SDL_TEXTINPUT) {
                if (input_length + strlen(event.text.text) < sizeof(input) - 1) {
                    strcat(input, event.text.text);
                    input_length = strlen(input);
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    strcpy(input, "default");
                    input_length = strlen(input);
                    done = 1;
                } else if (event.key.keysym.sym == SDLK_BACKSPACE && input_length > 0) {
                    input[--input_length] = '\0';
                } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                    done = 1;
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Color white = {255, 255, 255, 255};
        render_text(renderer, font, screen_width/2 - 100, screen_height/2 - 50, "Enter Username:", white);
        render_text(renderer, font, screen_width/2 - 100, screen_height/2, input, white);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    SDL_StopTextInput();
    DEBUG_PRINT(3, 2, "Username entered: %s", input);
    return strdup(input);
}

// Reads the blockchain file and returns the top score for the given username.
// If found, copies that block into topBlock (if not NULL) and returns its score; otherwise, returns -1.
int get_user_top_score(const char *username, ScoreBlock *topBlock) {
    FILE *fp = fopen(BLOCKCHAIN_FILE, "r");
    if (!fp) {
        DEBUG_PRINT(2, 1, "Blockchain file not found: %s", BLOCKCHAIN_FILE);
        return -1;
    }
    char line[2048];
    int topScore = -1;
    ScoreBlock temp;
    char prev_hash_buf[129] = {0};
    while (fgets(line, sizeof(line), fp)) {
        int ret = sscanf(line,
            "{\"username\":\"%49[^\"]\", \"score\":%d, \"timestamp\":%ld, \"proof_of_work\":\"%64[^\"]\", \"signature\":\"%512[^\"]\", \"prev_hash\":\"%128[^\"]\", \"nonce\":%u}",
            temp.username, &temp.score, &temp.timestamp, temp.proof_of_work, temp.signature, prev_hash_buf, &temp.nonce);
        if (ret == 7 && strcmp(temp.username, username) == 0) {
            prev_hash_buf[64] = '\0';
            strcpy(temp.prev_hash, prev_hash_buf);
            if (verify_score_signature(&temp, username, temp.signature)) {
                if (temp.score > topScore) {
                    topScore = temp.score;
                    if (topBlock)
                        *topBlock = temp;
                }
            } else {
                DEBUG_PRINT(2, 0, "Invalid signature for user %s in blockchain record", username);
            }
        }
    }
    fclose(fp);
    DEBUG_PRINT(2, 2, "Top score for user %s: %d", username, topScore);
    return topScore;
}

void game_loop() {
    Uint32 windowFlags = SDL_WINDOW_SHOWN; // | SDL_WINDOW_RESIZABLE;
    if (g_fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        DEBUG_PRINT(1, 0, "SDL_Init Error: %s", SDL_GetError());
        return;
    }
    if (TTF_Init() != 0) {
        DEBUG_PRINT(1, 0, "TTF_Init Error: %s", TTF_GetError());
        SDL_Quit();
        return;
    }

    int screen_width = 800, screen_height = 600;
    SDL_Window* win = SDL_CreateWindow("QuantumStriker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, windowFlags);
    if (!win) {
        DEBUG_PRINT(1, 0, "SDL_CreateWindow Error: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return;
    }

    if (g_fullscreen) {
        SDL_DisplayMode mode;
        if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
            screen_width = mode.w;
            screen_height = mode.h;
        }
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        DEBUG_PRINT(1, 0, "SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return;
    }
    
    TTF_Font* font = TTF_OpenFont("src/Arial.ttf", 16);
    if (!font) {
        DEBUG_PRINT(1, 0, "TTF_OpenFont Error: %s", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    Explosion explosions[MAX_EXPLOSIONS];
    for (int k = 0; k < MAX_EXPLOSIONS; k++) {
        explosions[k].lifetime = 0;
    }

    // Get or prompt username.
    char *orig_username = load_username();
    if (!orig_username) {
        orig_username = prompt_username(renderer, font, screen_width, screen_height);
        if (!orig_username)
            orig_username = strdup("default");
        save_username(orig_username);
    }
    if (!ensure_keypair(orig_username)) {
        DEBUG_PRINT(2, 0, "Key pair generation failed for user %s", orig_username);
    }
    DEBUG_PRINT(2, 3, "Starting game with username: %s", orig_username);

    char *username = NULL;
    if (g_dev_auto_mode) {
        // Append "DevAI" 
        username = malloc(strlen(orig_username) + 6); // "DevAi" (5) + '\0' (1)
        if (username) {
            strcpy(username, orig_username);
            strcat(username, "DevAI");
        } else {
            username = strdup(orig_username);
        }
        free(orig_username);
    } else {
        // not in dev auto mode. DONT DEREFERENCE A NULL POINTER
        username = orig_username;
    }

    // Initialize game objects.
    Player player;
    init_player(&player, screen_width, screen_height);
    if (g_dev_auto_mode) {
        player.health = AI_DEFAULT_HEALTH; 
        player.energy = AI_DEFAULT_ENERGY;
        DEBUG_PRINT(1, 3, "AI energy overridden to %f", player.energy);
        DEBUG_PRINT(1, 3, "AI health overridden to %d", player.health);
    }

    BulletPool bulletPool;
    init_bullet_pool(&bulletPool);
    
    Enemy enemies[MAX_ENEMIES];
    init_enemies(enemies);
    
    int enemiesKilled = 0;
    int score = 0;
    time_t startTime = time(NULL);
    int spawnTimer = 0;
    int frame = 0;
    int running = 1;
    SDL_Event e;
    
    // Ensure highscore directory exists.
    struct stat st = {0};
    if (stat("highscore", &st) == -1) {
        if (mkdir("highscore", 0755) == -1)
            DEBUG_PRINT(2, 0, "Error creating highscore directory: %s", strerror(errno));
        else
            DEBUG_PRINT(2, 3, "Highscore directory created");
    }
    
    srand((unsigned int)time(NULL));
    
    // Main game loop.
    while (running) {
        frame++;
        spawnTimer++;
        
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        float speedMultiplier = (keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL]) ? 2.0f : 1.0f;
        
        // Player controls:
        if (!g_dev_auto_mode) {
            if (keystate[SDL_SCANCODE_LEFT])
                rotate_player(&player, -2 * speedMultiplier);
            if (keystate[SDL_SCANCODE_RIGHT])
                rotate_player(&player, 2 * speedMultiplier);
            // Ship sizing keys:
            if (keystate[SDL_SCANCODE_DOWN])
                decrease_ship_size(&player);
            if (keystate[SDL_SCANCODE_UP])
                increase_ship_size(&player);
            if (keystate[SDL_SCANCODE_RSHIFT])
                reset_ship_size(&player);
            if (keystate[SDL_SCANCODE_W])
                thrust_player(&player);
            if (keystate[SDL_SCANCODE_S])
                reverse_thrust(&player);
            if (keystate[SDL_SCANCODE_A])
                strafe_left(&player);
            if (keystate[SDL_SCANCODE_D])
                strafe_right(&player);
            if (keystate[SDL_SCANCODE_SPACE]) {
                float tip_x, tip_y;
                get_ship_tip(&player, &tip_x, &tip_y);
                shoot_bullet(&bulletPool, tip_x, tip_y, player.angle, 0); // 0: player's bullet
            }
            if (keystate[SDL_SCANCODE_E])
                activate_shield(&player, 1);
            else
                activate_shield(&player, 0);
        } else {
            // In dev auto mode 
            DEBUG_PRINT(2, 2, "Entering dev_ai_control (screen %dx%d)", screen_width, screen_height);
            dev_ai_control(&player, enemies, &bulletPool, screen_width, screen_height);
        }
        
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
            else if (e.type == SDL_KEYDOWN &&
                     (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q)) {
                // Call pause menu.
                extern int pause_menu(SDL_Renderer*, TTF_Font*, int, int);
                int resume = pause_menu(renderer, font, screen_width, screen_height);
                if (!resume) {
                    player.health = 0;
                    DEBUG_PRINT(2, 0, "Quit selected from apuse menu. Game ended");
                } else {
                    DEBUG_PRINT(2, 3, "Resume selected from pause menu");
                }
            }
        }
        
        // Update game objects.
        update_player(&player);
        wrap_player_position(&player);
        update_shield_energy(&player);
        update_bullets(&bulletPool);
        
        float baseRate = 0.5f;
        float rateIncrease = score / 500.0f;
        float desiredSpawnRate = baseRate + rateIncrease;
        if (g_dev_auto_mode) {
            desiredSpawnRate *= AI_PROGRESS_MULTIPLIER;
        }
        if (desiredSpawnRate > 15.0f)
            desiredSpawnRate = 15.0f;
        float spawnIntervalSeconds = 1.0f / desiredSpawnRate;
        int spawnIntervalFrames = (int)(spawnIntervalSeconds / (FRAME_DELAY / 1000.0f));
        
        // Spawn enemy based on current score.
        if (spawnTimer > spawnIntervalFrames) {
            spawn_enemy(enemies, player.x, player.y, score);
            spawnTimer = 0;
            DEBUG_PRINT(3, 2, "Enemy spawned; spawnTimer reset");
        }
        
        float diffScale = 1.0f + (((score > 5000 ? 5000 : score) / 1000.0f)) * (g_dev_auto_mode ? AI_PROGRESS_MULTIPLIER : 1.0f);
        update_enemies(enemies, player.x, player.y, player.angle, diffScale, &bulletPool);
        
        // Process collisions between player's bullets and enemies.
        for (int i = 0; i < bulletPool.count; i++) {
            if (bulletPool.bullets[i].active && bulletPool.bullets[i].isEnemy == 0) {
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (enemies[j].active) {
                        float dx = bulletPool.bullets[i].x - enemies[j].x;
                        float dy = bulletPool.bullets[i].y - enemies[j].y;
                        float dist = sqrtf(dx * dx + dy * dy);
                        if (dist < COLLISIONTHRESHOLD) {
                            if (!(g_dev_auto_mode && AI_PIERCING_SHOT)) {
                                bulletPool.bullets[i].active = 0;
                            }
                            //enemies[j].health -= bulletPool.bullets[i].damage;
                            //DEBUG_PRINT(3, 2, "Player bullet hit enemy %d; new health = %d", j, enemies[j].health);
                            if (enemies[j].type == ENEMY_SHIELD && enemies[j].shieldActive) {
                                DEBUG_PRINT(3, 2, "Shielded enemy %d hit: no damage taken.", j);
                                bulletPool.bullets[i].active = 0;
                            } else {
                                enemies[j].health -= bulletPool.bullets[i].damage;
                                DEBUG_PRINT(3, 2, "Player bullet hit enemy %d; new health = %d", j, enemies[j].health);
                            }
                            if (enemies[j].health <= 0) {
                                for (int k = 0; k < MAX_EXPLOSIONS; k++) {
                                    if (explosions[k].lifetime <= 0) {
                                        explosions[k].x = enemies[j].x;
                                        explosions[k].y = enemies[j].y;
                                        explosions[k].radius = 5.0f;
                                        explosions[k].lifetime = 30; // lasts 30 frames
                                        if (enemies[j].type == ENEMY_SPLITTER) {
                                            split_enemy(enemies, j);
                                        }
                                        break;
                                    }
                                }
                                enemies[j].active = 0;
                                enemiesKilled++;
                                DEBUG_PRINT(3, 3, "Enemy %d destroyed; total enemies killed = %d", j, enemiesKilled);
                            }
                        }
                    }
                }
            }
        }
        
        // Process collisions between enemy bullets and the player.
        for (int i = 0; i < bulletPool.count; i++) {
            if (bulletPool.bullets[i].active && bulletPool.bullets[i].isEnemy == 1) {
                float dx = bulletPool.bullets[i].x - player.x;
                float dy = bulletPool.bullets[i].y - player.y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < 15) {
                    if (!player.shieldActive) {
                        player.health -= 1;
                        shakeTimer = 20;
                        shakeMagnitude = 10.0f;
                        DEBUG_PRINT(3, 2, "Player hit by enemy bullet; health reduced to %d", player.health);
                    } else {
                        DEBUG_PRINT(3, 2, "Enemy bullet blocked by shield.");
                    }
                    bulletPool.bullets[i].active = 0;
                }
            }
        }

        // Process collisions between enemies and the player.
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (enemies[j].active) {
                float dx = player.x - enemies[j].x;
                float dy = player.y - enemies[j].y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < 20) {
                    if (!player.shieldActive) {
                        player.health -= 1;
                        shakeTimer = 20;
                        shakeMagnitude = 10.0f;
                        DEBUG_PRINT(3, 2, "Player hit by enemy %d; health reduced to %d", j, player.health);
                    }
                    enemies[j].active = 0;
                }
            }
        }
        
        time_t now = time(NULL);
        score = (int)(now - startTime) + (enemiesKilled * 10);
        
        if (player.health <= 0) {
            DEBUG_PRINT(2, 2, "Game over. Using username: %s", username);
            
            ScoreBlock lastBlock = {0};
            int exists = get_last_block_for_user(username, &lastBlock);
            ScoreBlock newBlock = {0};
            strncpy(newBlock.username, username, sizeof(newBlock.username)-1);
            newBlock.score = score;
            newBlock.timestamp = now;
            if (!exists) {
                memset(newBlock.prev_hash, '0', HASH_STR_LEN - 1);
                newBlock.prev_hash[HASH_STR_LEN - 1] = '\0';
                add_score_block(&newBlock, NULL, DIFFICULTY);
                DEBUG_PRINT(2, 3, "Genesis block created for user %s", username);
            } else {
                strncpy(newBlock.prev_hash, lastBlock.proof_of_work, HASH_STR_LEN);
                newBlock.prev_hash[HASH_STR_LEN - 1] = '\0';
                add_score_block(&newBlock, &lastBlock, DIFFICULTY);
                DEBUG_PRINT(2, 3, "New block chained to last block for user %s", username);
            }
            
            if (!sign_score(&newBlock, username, newBlock.signature)) {
                DEBUG_PRINT(2, 0, "Failed to sign score block for user %s", username);
            } else {
                FILE *fp = fopen(BLOCKCHAIN_FILE, "a");
                if (fp) {
                    fprintf(fp,
                        "{\"username\":\"%s\", \"score\":%d, \"timestamp\":%ld, \"proof_of_work\":\"%s\", \"signature\":\"%s\", \"prev_hash\":\"%s\", \"nonce\":%u}\n",
                        newBlock.username, newBlock.score, newBlock.timestamp,
                        newBlock.proof_of_work, newBlock.signature, newBlock.prev_hash, newBlock.nonce);
                    fclose(fp);
                    DEBUG_PRINT(2, 3, "Score block appended for user %s", username);
                } else {
                    DEBUG_PRINT(2, 0, "Failed to open blockchain file for appending");
                }
            }
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_Color white = {255, 255, 255, 255};
            render_text(renderer, font, screen_width/2 - 100, screen_height/2 - 80, "GAME OVER", white);
            char buffer[150];
            sprintf(buffer, "Time Survived: %ld seconds", now - startTime);
            render_text(renderer, font, screen_width/2 - 120, screen_height/2 - 50, buffer, white);
            sprintf(buffer, "Enemies Killed: %d", enemiesKilled);
            render_text(renderer, font, screen_width/2 - 120, screen_height/2 - 30, buffer, white);
            sprintf(buffer, "Score: %d", score);
            render_text(renderer, font, screen_width/2 - 120, screen_height/2 - 10, buffer, white);
            render_text(renderer, font, screen_width/2 - 120, screen_height/2 + 10, "Score submitted securely!", white);
            SDL_RenderPresent(renderer);
            SDL_Delay(3000);
            running = 0;
            break;
        }
        
        float cam_x = player.x - screen_width/2;
        float cam_y = player.y - screen_height/2;
        if (shakeTimer > 0) {
            cam_x += (rand() % ((int)(shakeMagnitude * 2) + 1)) - shakeMagnitude;
            cam_y += (rand() % ((int)(shakeMagnitude * 2) + 1)) - shakeMagnitude;
            shakeTimer--;
        }
        
        draw_background(renderer, cam_x, cam_y, screen_width, screen_height);
        SDL_Color white = {255, 255, 255, 255};
        char hud[200];
        if (g_dev_auto_mode) {
            sprintf(hud, "Health: %d  Energy: %.1f  Score: %d  X: %.1f  Y: %.1f  Angle: %.1f", 
                    player.health, player.energy, score, player.x, player.y, player.angle);
        } else {
            sprintf(hud, "Health: %d  Energy: %.1f  Score: %d  X: %.1f  Y: %.1f  Angle: %.1f",
                    player.health, player.energy, score, player.x, player.y, player.angle);
        }
        render_text(renderer, font, 10, 10, hud, white);
        draw_bullets(&bulletPool, renderer, cam_x, cam_y);
        draw_enemies(enemies, renderer, cam_x, cam_y);
        draw_player(&player, renderer, screen_width/2, screen_height/2);

        for (int k = 0; k < MAX_EXPLOSIONS; k++) {
            if (explosions[k].lifetime > 0) {
                explosions[k].radius += 1.0f;  // Expand explosion radius
                int alpha = (int)(255 * ((float)explosions[k].lifetime / 30.0f)); // Fade effect
                filledCircleRGBA(renderer, (int)(explosions[k].x - cam_x), (int)(explosions[k].y - cam_y),
                                 (int)explosions[k].radius, 255, 165, 0, alpha);
                explosions[k].lifetime--;
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(FRAME_DELAY);
        if (g_exit_requested) {
            player.health = 0;
        }
    }
    
    free_bullet_pool(&bulletPool);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    
    free(username);
}

