#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"       // Make sure HIGHSCORE_FLAG_MAX_ENTRY_NUMBER is defined here.
#include "blockchain.h"   // For ScoreBlock structure and HASH_STR_LEN.
#include "signature.h"    // For verify_score_signature

#define MAX_BLOCKS 1000  // Maximum number of blocks to read from the blockchain file

// Helper function to strip "DevAI" suffix from a username, if present.
static void strip_devai_suffix(char *username, size_t max_len) {
    (void)max_len; // suppressed warning; 
                   // for future implementation with buffer checks
    const char *suffix = "DevAI";
    size_t ulen = strlen(username);
    size_t slen = strlen(suffix);
    
    if (ulen >= slen && strcmp(username + ulen - slen, suffix) == 0) {
        username[ulen - slen] = '\0';
    }
}

// Comparison function for qsort: descending order by score.
static int compare_score(const void *a, const void *b) {
    const ScoreBlock *A = (const ScoreBlock *)a;
    const ScoreBlock *B = (const ScoreBlock *)b;
    return B->score - A->score;
}

// Reads the blockchain file, validates each block using the base username (with "DevAI" stripped),
// and stores valid entries in the blocks array.
static int read_and_validate_blocks(ScoreBlock *blocks) {
    FILE *fp = fopen("highscore/blockchain.txt", "r");
    if (!fp) {
        printf("Blockchain file not found.\n");
        return 0;
    }

    int count = 0;
    char line[2048];

    while (fgets(line, sizeof(line), fp)) {
        if (strlen(line) == 0) continue;

        ScoreBlock block;
        char prev_hash_buf[HASH_STR_LEN * 2] = {0}; // temporary buffer

        int ret = sscanf(line,
            "{\"username\":\"%49[^\"]\", \"score\":%d, \"timestamp\":%ld, \"proof_of_work\":\"%64[^\"]\", \"signature\":\"%512[^\"]\", \"prev_hash\":\"%64[^\"]\", \"nonce\":%u}",
            block.username, &block.score, &block.timestamp,
            block.proof_of_work, block.signature, prev_hash_buf, &block.nonce);

        if (ret != 7) continue;

        // Ensure prev_hash is properly null-terminated
        strncpy(block.prev_hash, prev_hash_buf, HASH_STR_LEN - 1);
        block.prev_hash[HASH_STR_LEN - 1] = '\0';

        // Copy the username and strip "DevAI" for verification
        char base_username[128];
        strncpy(base_username, block.username, sizeof(base_username) - 1);
        base_username[sizeof(base_username) - 1] = '\0';
        strip_devai_suffix(base_username, sizeof(base_username));

        // Validate digital signature using the base username
        if (!verify_score_signature(&block, base_username, block.signature)) continue;

        blocks[count++] = block;
        if (count >= MAX_BLOCKS) break;
    }

    fclose(fp);
    return count;
}

// Displays the high score table.
void display_highscores(void) {
    ScoreBlock blocks[MAX_BLOCKS];
    int count = read_and_validate_blocks(blocks);

    if (count == 0) {
        printf("No valid blockchain entries found.\n");
        return;
    }

    // Convert usernames to base form (without "DevAI") for display
    for (int i = 0; i < count; i++) {
        char base_username[128];
        strncpy(base_username, blocks[i].username, sizeof(base_username) - 1);
        base_username[sizeof(base_username) - 1] = '\0';
        strip_devai_suffix(base_username, sizeof(base_username));

        strncpy(blocks[i].username, base_username, sizeof(blocks[i].username) - 1);
        blocks[i].username[sizeof(blocks[i].username) - 1] = '\0';
    }

    // Sort valid blocks by score in descending order
    qsort(blocks, count, sizeof(ScoreBlock), compare_score);

    // Limit the number of entries displayed
    int display_count = (count < HIGHSCORE_FLAG_MAX_ENTRY_NUMBER) ? count : HIGHSCORE_FLAG_MAX_ENTRY_NUMBER;

    // Print header
    printf("+----------------------+------------+---------------------+\n");
    printf("| Username             | High Score | Timestamp           |\n");
    printf("+----------------------+------------+---------------------+\n");

    // Display each entry
    for (int i = 0; i < display_count; i++) {
        char timestr[64];
        time_t t = blocks[i].timestamp;
        struct tm *tm_info = localtime(&t);
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

        printf("| %-20s | %10d | %-19s |\n", blocks[i].username, blocks[i].score, timestr);
    }

    printf("+----------------------+------------+---------------------+\n");
}

