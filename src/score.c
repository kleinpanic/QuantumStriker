#include "score.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define USERNAME_FILE ".username"
#define HIGHSCORE_DIR "highscore"
#define BLOCKCHAIN_FILE "highscore/blockchain.txt"

int load_highscore_for_username(const char *username) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s_highscore.txt", HIGHSCORE_DIR, username);
    FILE *file = fopen(path, "r");
    if (!file) {
        DEBUG_PRINT(2, 1, "Highscore file not found for user %s at path %s", username, path);
        return 0;
    }
    int highscore = 0;
    fscanf(file, "%d", &highscore);
    fclose(file);
    DEBUG_PRINT(2, 2, "Loaded highscore for user %s: %d", username, highscore);
    return highscore;
}

void save_highscore_for_username(const char *username, int score) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s_highscore.txt", HIGHSCORE_DIR, username);
    FILE *file = fopen(path, "w");
    if (file) {
        fprintf(file, "%d", score);
        fclose(file);
        DEBUG_PRINT(2, 3, "Saved highscore for user %s: %d", username, score);
    } else {
        DEBUG_PRINT(2, 0, "Failed to open highscore file for writing for user %s at path %s", username, path);
    }
}

char* load_username() {
    FILE *file = fopen(USERNAME_FILE, "r");
    if (!file) {
        DEBUG_PRINT(2, 1, "Username file %s not found", USERNAME_FILE);
        return NULL;
    }
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        DEBUG_PRINT(2, 0, "Failed to read username from %s", USERNAME_FILE);
        fclose(file);
        return NULL;
    }
    fclose(file);
    buffer[strcspn(buffer, "\n")] = 0;
    DEBUG_PRINT(2, 2, "Username loaded: %s", buffer);
    return strdup(buffer);
}

void save_username(const char *username) {
    FILE *file = fopen(USERNAME_FILE, "w");
    if (file) {
        fprintf(file, "%s", username);
        fclose(file);
        DEBUG_PRINT(2, 3, "Username saved: %s", username);
    } else {
        DEBUG_PRINT(2, 0, "Failed to open username file %s for writing", USERNAME_FILE);
    }
}

/* NEW FUNCTION: get_last_block_for_user
   Scans the blockchain file and returns the block with the highest timestamp
   for the given username. Copies that block into lastBlock and returns 1 if found,
   or returns 0 if no block exists for that user.
*/
int get_last_block_for_user(const char *username, ScoreBlock *lastBlock) {
    FILE *fp = fopen(BLOCKCHAIN_FILE, "r");
    if (!fp) {
        DEBUG_PRINT(2, 1, "Blockchain file %s not found.", BLOCKCHAIN_FILE);
        return 0;
    }
    char line[2048];
    int found = 0;
    ScoreBlock temp;
    char prev_hash_buf[129] = {0};  // Buffer for prev_hash (128 chars + null)
    while (fgets(line, sizeof(line), fp)) {
        int ret = sscanf(line,
            "{\"username\":\"%49[^\"]\", \"score\":%d, \"timestamp\":%ld, \"proof_of_work\":\"%64[^\"]\", \"signature\":\"%512[^\"]\", \"prev_hash\":\"%128[^\"]\", \"nonce\":%u}",
            temp.username, &temp.score, &temp.timestamp, temp.proof_of_work, temp.signature, prev_hash_buf, &temp.nonce);
        if (ret == 7 && strcmp(temp.username, username) == 0) {
            prev_hash_buf[64] = '\0';
            strcpy(temp.prev_hash, prev_hash_buf);
            if (!found || temp.timestamp > lastBlock->timestamp) {
                *lastBlock = temp;  // copy the block structure
                found = 1;
            }
        }
    }
    fclose(fp);
    if (found)
        DEBUG_PRINT(2, 2, "Most recent block for user %s found with timestamp %ld", username, lastBlock->timestamp);
    else
        DEBUG_PRINT(2, 1, "No block found for user %s", username);
    return found;
}

