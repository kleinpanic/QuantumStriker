#include "blockchain.h"
#include "encryption.h"   // for hash_score function
#include "debug.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define HASH_STR_LEN 65  // assuming SHA-256 hex string length (64 + null terminator)

// Helper: Check if a hash (hex string) meets difficulty requirement (leading zeros).
static int hash_meets_difficulty(const char *hash, int difficulty) {
    for (int i = 0; i < difficulty; i++) {
        if (hash[i] != '0')
            return 0;
    }
    return 1;
}

// Compute hash over block data (excluding proof_of_work, signature, and nonce)
// and write the hex digest into output_hash.
void compute_block_hash(const ScoreBlock *block, char *output_hash) {
    char buffer[512] = {0}; // zero initialize to avoid garbage
    // Build a string from the block fields.
    // Ensure we print the timestamp as a long and the nonce as unsigned int.
    snprintf(buffer, sizeof(buffer), "%s|%d|%ld|%s|%u",
             block->username,
             block->score,
             block->timestamp,
             block->prev_hash,
             block->nonce);
    // Debug: output the buffer being hashed.
    DEBUG_PRINT(2, 2, "compute_block_hash: buffer = \"%s\"", buffer);
    hash_score(buffer, output_hash);
    DEBUG_PRINT(2, 2, "compute_block_hash: computed hash = %s", output_hash);
}

// Computes valid proof-of-work for the new block.
// Adjusts block->nonce until the computed hash meets the specified difficulty.
static void compute_proof_of_work(ScoreBlock *block, int difficulty) {
    char hash[HASH_STR_LEN] = {0};
    block->nonce = 0;
    while (1) {
        compute_block_hash(block, hash);
        // Debug: log each nonce and its corresponding hash.
        DEBUG_PRINT(2, 2, "Proof-of-Work: nonce = %u, hash = %s", block->nonce, hash);
        if (hash_meets_difficulty(hash, difficulty)) {
            strncpy(block->proof_of_work, hash, HASH_STR_LEN - 1);
            block->proof_of_work[HASH_STR_LEN - 1] = '\0';
            DEBUG_PRINT(2, 3, "Valid PoW found: nonce = %u, hash = %s", block->nonce, hash);
            break;
        }
        block->nonce++;
    }
}

// Adds a new block to the blockchain.
// Fills in prev_hash and computes proof-of-work.
void add_score_block(ScoreBlock *newBlock, const ScoreBlock *prev, int difficulty) {
    if (prev) {
        // Copy only HASH_STR_LEN-1 characters and ensure null termination.
        strncpy(newBlock->prev_hash, prev->proof_of_work, HASH_STR_LEN - 1);
        newBlock->prev_hash[HASH_STR_LEN - 1] = '\0';
    } else {
        // Genesis block: set prev_hash to all zeros.
        memset(newBlock->prev_hash, '0', HASH_STR_LEN - 1);
        newBlock->prev_hash[HASH_STR_LEN - 1] = '\0';
    }
    // Set the timestamp if not already set.
    if (newBlock->timestamp == 0)
        newBlock->timestamp = time(NULL);

    DEBUG_PRINT(2, 2, "add_score_block: Before PoW: Username: %s, Score: %d, Timestamp: %ld, Prev_hash: %s",
                newBlock->username, newBlock->score, newBlock->timestamp, newBlock->prev_hash);

    // Compute proof-of-work; this updates newBlock->nonce and newBlock->proof_of_work.
    compute_proof_of_work(newBlock, difficulty);

    DEBUG_PRINT(2, 2, "add_score_block: After PoW: Proof_of_work: %s, Nonce: %u", newBlock->proof_of_work, newBlock->nonce);
}

// Verifies the blockchain integrity.
int verify_blockchain(const ScoreBlock *chain, int count, int difficulty) {
    char recomputed_hash[HASH_STR_LEN] = {0};
    for (int i = 0; i < count; i++) {
        const ScoreBlock *block = &chain[i];
        compute_block_hash(block, recomputed_hash);
        DEBUG_PRINT(2, 2, "verify_blockchain: Block %d: Stored PoW: %s", i, block->proof_of_work);
        DEBUG_PRINT(2, 2, "verify_blockchain: Block %d: Recomputed hash: %s", i, recomputed_hash);
        if (strncmp(block->proof_of_work, recomputed_hash, HASH_STR_LEN) != 0) {
            DEBUG_PRINT(2, 0, "Block %d: invalid proof-of-work hash.", i);
            return 0;
        }
        if (!hash_meets_difficulty(block->proof_of_work, difficulty)) {
            DEBUG_PRINT(2, 0, "Block %d: proof-of-work does not meet difficulty.", i);
            return 0;
        }
        if (i > 0) {
            if (strncmp(block->prev_hash, chain[i-1].proof_of_work, HASH_STR_LEN) != 0) {
                DEBUG_PRINT(2, 0, "Block %d: previous hash does not match", i);
                return 0;
            }
        }
    }
    return 1;
}

