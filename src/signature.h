#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "blockchain.h"
#include <stddef.h>

// Signs the block data using the provided private key.
// The signature is output as a hex string into 'signature' (which has space for at least SIG_STR_LEN bytes).
// Returns 1 on success, 0 on failure.
int sign_score(const ScoreBlock *block, const char *username, char *signature);

// Verifies the block's signature using the public key associated with username.
// Returns 1 if the signature is valid, 0 otherwise.
int verify_score_signature(const ScoreBlock *block, const char *username, const char *signature);

#endif // SIGNATURE_H

