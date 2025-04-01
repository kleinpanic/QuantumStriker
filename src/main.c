#include "game.h"
#include "version.h"
#include "score.h"
#include "debug.h"
#include "config.h"
#include "enemy.h"
int g_fullscreen = 0;
int g_testing_mode = 0;
int g_dev_auto_mode = 0;
int g_forced_enemy_type = -1; // -1 means "not set"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

volatile sig_atomic_t g_exit_requested = 0;

void handle_sigint(int sig) {
    (void)sig; // Unused parameter.
    g_exit_requested = 1;
    DEBUG_PRINT(0, 3, "SIGINT received: exiting game loop");
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    // Process command-line arguments.
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0) {
            DEBUG_PRINT(0, 3, "Version flag activated.");
            DEBUG_PRINT(3, 3, "Version flag activated. Version number will be read from version.h file, and status will return 0");
            printf("Quantum Striker Version %s\n", QUANTUM_STRIKER_VERSION);
            return 0;
        } else if (strcmp(argv[i], "--help") == 0) {
            DEBUG_PRINT(2, 3, "Help flag active");
            printf("Usage: %s [--version] [--help] [--debug <1-3>] [--fullscreen] [--highscores]\n", argv[0]);
            printf("\n");
            printf("  --version    Print the version number\n");
            printf("  --help       Show this help message\n");
            printf("  --debug      Enable debug mode with a level (1-3)\n");
            printf("  --fullscreen Fullscreen mode \n");
            printf("  --highscores Display a table of all high scores\n");
            return 0;
        } else if (strcmp(argv[i], "--debug") == 0) {
            g_debug_enabled = 1;
            if (i + 1 < argc) {
                int level = atoi(argv[i+1]);
                if (level >= 1 && level <= 3) {
                    g_debug_level = level;
                }
                i++; // Skip the level parameter.
            }
            DEBUG_PRINT(1, 3, "Debug mode on.");
            DEBUG_PRINT(2, 3, "Debug mode enabled with level %d", g_debug_level);
            DEBUG_PRINT(3, 3, "Debug mode enabled with level %d. Highest debug enabled.", g_debug_level);
        } else if (strcmp(argv[i], "--fullscreen") == 0) {

            g_fullscreen = 1;
            DEBUG_PRINT(0, 3, "Fullscreen flag activated.");
        } else if (strcmp(argv[i], "--highscores") == 0) {
            #include "highscores.h"
            display_highscores();
            return 0;
        } else if (strcmp(argv[i], "--development") == 0) {
            // Check if a Sub-argument is provided. 
            if (i + 1 >= argc) {
                DEBUG_PRINT(0, 1, "Usage for --development:\n");
                DEBUG_PRINT(0, 1, "     auto: Enables automatic playstyle for dev mode\n");
                DEBUG_PRINT(0, 1, "     testing: Calls Testing Options\n");
                return 1;
            } else {
                // a subargument has been provided! yay
                if (strcmp(argv[i+1], "auto") == 0) {
                    g_dev_auto_mode = 1;
                    DEBUG_PRINT(0, 3, "Development mode activated w/ auto option");
                    i++; //skip the "auto" argument and continue
                } else if (strcmp(argv[i+1], "testing") == 0) {
                    g_testing_mode = 1;
                    DEBUG_PRINT(0, 2, "Calling Testing Mode in src/main.c");
                    if (i + 2 >= argc) {
                        DEBUG_PRINT(0, 1, "Usage for Testing Option:\n");
                        return 1;
                    } else { 
                        if (strcmp(argv[i+2], "enemy") == 0) {
                            DEBUG_PRINT(0, 2, "Calling Enemy Testing Option");
                            //i++;
                            if (i + 3 >= argc) {
                                DEBUG_PRINT(0, 1, "Usage for Enemy flag:\n");
                                return 1;
                            } else {
                                int forced = atoi(argv[i+3]);
                                switch (forced) {
                                    case 1:
                                        g_forced_enemy_type = ENEMY_BASIC;
                                        DEBUG_PRINT(0, 2, "Spawning only default enemy");
                                        break;
                                    case 2: 
                                        g_forced_enemy_type = ENEMY_SHOOTER;
                                        DEBUG_PRINT(0, 2, "Spawning only shooter enemy");
                                        break;
                                    case 3:
                                        g_forced_enemy_type = ENEMY_TANK;
                                        DEBUG_PRINT(0, 2, "Spawning only shooter enemy");
                                        break;
                                    case 4:
                                        g_forced_enemy_type = ENEMY_EVASIVE;
                                        DEBUG_PRINT(0, 2, "Spawning only shooter enemy");
                                        break;
                                    case 5:
                                        g_forced_enemy_type = ENEMY_FAST;
                                        DEBUG_PRINT(0, 2, "Spawning only shooter enemy");
                                        break;
                                    case 6:
                                        g_forced_enemy_type = ENEMY_SPLITTER;
                                        DEBUG_PRINT(0, 2, "Spawning only shooter enemy");
                                        break;
                                    case 7:
                                        g_forced_enemy_type = ENEMY_STEALTH;
                                        DEBUG_PRINT(0, 2, "Spawning only shooter enemy");
                                        break;
                                    default:
                                        DEBUG_PRINT(0, 1, "Invalid Usage\n");
                                        return 1;
                                }
                            }
                        } else {
                            DEBUG_PRINT(0, 1, "Invalid Testing Option\n");
                            return 1;
                        }
                    }
                    i += 3; // skip the subarguments: "testing" "enemy" and the enemy number.
                } else {
                    // Invalide sub argument was passed. Print error
                    DEBUG_PRINT(0, 1, "Invalid Usage of --development:\n");
                    printf("unknown argument : '%s'\n", argv[i+1]);
                    DEBUG_PRINT(0, 1, "Usage for --development:\n");
                    DEBUG_PRINT(0, 1, "     auto: Enables automatic playstyle for dev mode\n");
                    DEBUG_PRINT(0, 1, "     testing: Calls testing options\n");
                    return 1;
                }
            }
        } else {
            printf("Unknown option: %s\nTry '--help' for usage.\n", argv[i]);
            return 1;
        }
    }
    // No command-line options provided; run the game.
    game_loop();
    return 0;
}

