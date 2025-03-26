#include "game.h"
#include "version.h"
#include "score.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    // Process command-line arguments.
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0) {
            DEBUG_PRINT(2, 3, "Version flag activated.");
            DEBUG_PRINT(3, 3, "Version flag activated. Version number will be read from version.h file, and status will return 0");
            printf("Nebula Nexus version %s\n", NEBULA_NEXUS_VERSION);
            return 0;
        } else if (strcmp(argv[i], "--help") == 0) {
            DEBUG_PRINT(2, 3, "Help flag active");
            printf("Usage: %s [--version] [--help] [--debug <1-3>] [--fullscreen] [--highscores]\n", argv[0]);
            printf("\n");
            printf("  --version    Print the version number\n");
            printf("  --help       Show this help message\n");
            printf("  --debug      Enable debug mode with a level (1-3)\n");
            printf("  --fullscreen Fullscreen mode (feature to be implemented later)\n");
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
            DEBUG_PRINT(2, 3, "Fullscreen flag active (not implemented)");
            printf("Fullscreen support will be implemented in a future release.\n");
            return 0;
        } else if (strcmp(argv[i], "--highscores") == 0) {
            DEBUG_PRINT(2, 3, "Highscores flag active");
            DIR *dir = opendir("highscore");
            if (!dir) {
                printf("No highscore directory found.\n");
                return 0;
            }
            printf("+----------------------+------------+\n");
            printf("| Username             | High Score |\n");
            printf("+----------------------+------------+\n");
            struct dirent *entry;
            char filepath[512];
            while ((entry = readdir(dir)) != NULL) {
                if (strstr(entry->d_name, "_highscore.txt") != NULL) {
                    char *suffix = strstr(entry->d_name, "_highscore.txt");
                    size_t len = suffix - entry->d_name;
                    char username[101] = {0};
                    strncpy(username, entry->d_name, len);
                    username[len] = '\0';
                    snprintf(filepath, sizeof(filepath), "highscore/%s", entry->d_name);
                    FILE *file = fopen(filepath, "r");
                    int hs = 0;
                    if (file) {
                        fscanf(file, "%d", &hs);
                        fclose(file);
                    }
                    printf("| %-20s | %10d |\n", username, hs);
                }
            }
            printf("+----------------------+------------+\n");
            closedir(dir);
            return 0;
        } else {
            printf("Unknown option: %s\nTry '--help' for usage.\n", argv[i]);
            return 1;
        }
    }
    // No command-line options provided; run the game.
    game_loop();
    return 0;
}

