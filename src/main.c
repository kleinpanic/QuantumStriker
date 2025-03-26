#include "game.h"
#include "version.h"
#include "score.h"
#include "debug.h"
int g_debug_level = 0; // default: no debug messages

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0) {
            printf("Nebula Nexus version %s\n", NEBULA_NEXUS_VERSION);
            return 0;
        } else if (strcmp(argv[1], "--help") == 0) {
            printf("Usage: %s [--version] [--help] [--debug] [--fullscreen] [--highscores]\n", argv[0]);
            printf("\n");
            printf("  --version    Print the version number\n");
            printf("  --help       Show this help message\n");
            printf("  --debug      Debug mode (feature to be implemented later)\n");
            printf("  --fullscreen Fullscreen mode (feature to be implemented later)\n");
            printf("  --highscores Display a table of all high scores\n");
            return 0;
        } else if (strcmp(argv[1], "--debug") == 0) {
            int level = 1; // default debug level if no number is provided
            // Check if next argument exists and is a number.
            if (i + 1 < argc) {
                int tmp = atoi(argv[i+1]);
                if (tmp >= 1 && tmp <= 3) {
                    level = tmp;
                    i++; // Skip the number argument
                }
            }
            g_debug_level = level;
            printf("Debug mode enabled. Level: %d\n", g_debug_level);
        } else if (strcmp(argv[1], "--fullscreen") == 0) {
            printf("Fullscreen support will be implemented in a future release.\n");
            return 0;
        } else if (strcmp(argv[1], "--highscores") == 0) {
            // Open the "highscore" directory and print a table.
            DIR *dir = opendir("highscore");
            if (!dir) {
                printf("No highscore directory found.\n");
                return 0;
            }
            printf("+----------------------+------------+\n");
            printf("| Username             | High Score |\n");
            printf("+----------------------+------------+\n");
            struct dirent *entry;
            // Increase buffer size to prevent truncation warnings.
            char filepath[512];
            while ((entry = readdir(dir)) != NULL) {
                if (strstr(entry->d_name, "_highscore.txt") != NULL) {
                    // Extract username by stripping the suffix.
                    char *suffix = strstr(entry->d_name, "_highscore.txt");
                    size_t len = suffix - entry->d_name;
                    char username[101] = {0};
                    strncpy(username, entry->d_name, len);
                    username[len] = '\0';
                    // Construct file path using the new buffer size.
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
            printf("Unknown option: %s\nTry '--help' for usage.\n", argv[1]);
            return 1;
        }
    }
    // No command-line options provided; run the game.
    game_loop();
    return 0;
}

