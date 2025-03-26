#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <time.h>
#include "debug.h"

#define USERNAME_MAX 50
#define HASH_STR_LEN 65     // 64 hex digits + null terminator
#define SIG_STR_LEN 513     // Example size for RSA signature in hex (256 + '\0')
#define NONCE_STR_LEN 16    // Nonce field as string (if used)

// Structure representing a high score block in the chain.
typedef struct {
    char username[USERNAME_MAX];
    int score;
    time_t timestamp;
    char proof_of_work[HASH_STR_LEN];  // SHA-256 hash in hex
    char signature[SIG_STR_LEN];       // Digital signature in hex
    char prev_hash[HASH_STR_LEN];      // Previous block's hash
    unsigned int nonce;                // Nonce used in proof-of-work
} ScoreBlock;

// Adds a new block to the blockchain array.
// prev is the previous block; if NULL, this is the genesis block.
void add_score_block(ScoreBlock *newBlock, const ScoreBlock *prev, int difficulty);

// Verifies the entire blockchain. Returns 1 if valid, 0 otherwise.
int verify_blockchain(const ScoreBlock *chain, int count, int difficulty);

// Computes SHA-256 over a block (excluding the proof_of_work, signature, and nonce fields)
// and writes the hex digest into output_hash. (Used in PoW.)
void compute_block_hash(const ScoreBlock *block, char *output_hash);

#endif // BLOCKCHAIN_H

