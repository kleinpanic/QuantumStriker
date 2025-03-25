#include "score.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define USERNAME_FILE ".username"
#define HIGHSCORE_DIR "highscore"

int load_highscore_for_username(const char *username) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s_highscore.txt", HIGHSCORE_DIR, username);
    FILE *file = fopen(path, "r");
    if (!file) return 0;
    int highscore = 0;
    fscanf(file, "%d", &highscore);
    fclose(file);
    return highscore;
}

void save_highscore_for_username(const char *username, int score) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s_highscore.txt", HIGHSCORE_DIR, username);
    FILE *file = fopen(path, "w");
    if (file) {
        fprintf(file, "%d", score);
        fclose(file);
    }
}

char* load_username() {
    FILE *file = fopen(USERNAME_FILE, "r");
    if (!file) return NULL;
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        return NULL;
    }
    fclose(file);
    buffer[strcspn(buffer, "\n")] = 0;
    return strdup(buffer);
}

void save_username(const char *username) {
    FILE *file = fopen(USERNAME_FILE, "w");
    if (file) {
        fprintf(file, "%s", username);
        fclose(file);
    }
}

