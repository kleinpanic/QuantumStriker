#include "game.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "background.h"
#include "score.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FRAME_DELAY 15   // milliseconds per frame

// Helper function to render text using SDL_ttf.
void render_text(SDL_Renderer* renderer, TTF_Font* font, int x, int y, const char* text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
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
                    // If Esc is pressed during prompt, set username to "default"
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
        SDL_Delay(16); // approx. 60 FPS
    }
    
    SDL_StopTextInput();
    return strdup(input);
}

void game_loop() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return;
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init Error: %s", TTF_GetError());
        SDL_Quit();
        return;
    }
    
    SDL_Window* win = SDL_CreateWindow("QuantumStriker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!win) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return;
    }
    
    // Load font from src directory.
    TTF_Font* font = TTF_OpenFont("src/Arial.ttf", 16);
    if (!font) {
        SDL_Log("TTF_OpenFont Error: %s", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return;
    }
    
    int screen_width = 800;
    int screen_height = 600;
    Player player;
    init_player(&player, screen_width, screen_height);
    
    // Initialize dynamic bullet pool.
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
        mkdir("highscore", 0755);
    }
    
    srand((unsigned int)time(NULL));
    
    while (running) {
        frame++;
        spawnTimer++;
        
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        float speedMultiplier = (keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL]) ? 2.0f : 1.0f;
        
        if (keystate[SDL_SCANCODE_LEFT])
            rotate_player(&player, -2 * speedMultiplier);
        if (keystate[SDL_SCANCODE_RIGHT])
            rotate_player(&player, 2 * speedMultiplier);
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
            shoot_bullet(&bulletPool, tip_x, tip_y, player.angle);
        }
        if (keystate[SDL_SCANCODE_E])
            activate_shield(&player, 1);
        else
            activate_shield(&player, 0);
        
        // Check for quit keys (Q and Escape) in the event loop.
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
            else if (e.type == SDL_KEYDOWN &&
                     (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q))
                running = 0;
        }
        
        update_player(&player);
        update_shield_energy(&player);
        update_bullets(&bulletPool);
        
        // Increase spawn rate with score.
        float baseRate = 0.5f;
        float rateIncrease = score / 500.0f;
        float desiredSpawnRate = baseRate + rateIncrease;
        if (desiredSpawnRate > 10.0f)
            desiredSpawnRate = 10.0f;
        float spawnIntervalSeconds = 1.0f / desiredSpawnRate;
        int spawnIntervalFrames = (int)(spawnIntervalSeconds / (FRAME_DELAY / 1000.0f));
        
        if (spawnTimer > spawnIntervalFrames) {
            spawn_enemy(enemies, player.x, player.y, 1.0f);
            spawnTimer = 0;
        }
        
        float difficulty = 1.0f + ((score > 5000 ? 5000 : score) / 1000.0f);
        update_enemies(enemies, player.x, player.y, difficulty);
        
        for (int i = 0; i < bulletPool.count; i++) {
            if (bulletPool.bullets[i].active) {
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (enemies[j].active) {
                        float dx = bulletPool.bullets[i].x - enemies[j].x;
                        float dy = bulletPool.bullets[i].y - enemies[j].y;
                        float dist = sqrtf(dx * dx + dy * dy);
                        if (dist < 15) {
                            enemies[j].health -= 1;
                            bulletPool.bullets[i].active = 0;
                            if (enemies[j].health <= 0) {
                                enemies[j].active = 0;
                                enemiesKilled++;
                            }
                        }
                    }
                }
            }
        }
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (enemies[j].active) {
                float dx = player.x - enemies[j].x;
                float dy = player.y - enemies[j].y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < 20) {
                    if (!player.shieldActive)
                        player.health -= 1;
                    enemies[j].active = 0;
                }
            }
        }
        
        time_t now = time(NULL);
        score = (int)(now - startTime) + (enemiesKilled * 10);
        
        if (player.health <= 0) {
            // At game over, prompt for username via SDL window.
            char *username = load_username();
            if (!username) {
                username = prompt_username(renderer, font, screen_width, screen_height);
                if (!username) {
                    username = strdup("default");
                }
                save_username(username);
            }
            
            int highscore = load_highscore_for_username(username);
            
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
            // Only update high score file if the username is "default"
            if (strcmp(username, "default") == 0) {
                if (score > highscore) {
                    save_highscore_for_username(username, score);
                    render_text(renderer, font, screen_width/2 - 120, screen_height/2 + 10, "New High Score!", white);
                } else {
                    sprintf(buffer, "High Score: %d", highscore);
                    render_text(renderer, font, screen_width/2 - 120, screen_height/2 + 10, buffer, white);
                }
            } else {
                sprintf(buffer, "High Score: %d", highscore);
                render_text(renderer, font, screen_width/2 - 120, screen_height/2 + 10, buffer, white);
            }
            SDL_RenderPresent(renderer);
            SDL_Delay(3000);
            free(username);
            running = 0;
            break;
        }
        
        float cam_x = player.x - screen_width/2;
        float cam_y = player.y - screen_height/2;
        
        draw_background(renderer, cam_x, cam_y, screen_width, screen_height);
        char hud[200];
        sprintf(hud, "Health: %d  Energy: %.1f  Score: %d  X: %.1f  Y: %.1f  Angle: %.1f", 
                player.health, player.energy, score, player.x, player.y, player.angle);
        SDL_Color white = {255, 255, 255, 255};
        render_text(renderer, font, 10, 10, hud, white);
        draw_bullets(&bulletPool, renderer, cam_x, cam_y);
        draw_enemies(enemies, renderer, cam_x, cam_y);
        draw_player(&player, renderer, screen_width/2, screen_height/2);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(FRAME_DELAY);
    }
    
    free_bullet_pool(&bulletPool);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
}

