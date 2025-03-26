#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stddef.h>
#include "debug.h"

#ifndef USERNAME_FILE
#define USERNAME_FILE ".username"
#endif

// Computes a SHA-256 hash over the input data (a null-terminated string)
// and writes the hex digest (65 bytes) into output_hash.
void hash_score(const char *data, char *output_hash);

// Generates a new RSA key pair for the given username.
// The private key is stored in the .username file (private, not pushed),
// and the public key is stored in the highscore/public_keys directory.
// Returns 1 on success, 0 on failure.
int generate_keypair(const char *username);

// Loads the private key associated with the given username from the .username file.
// Returns a pointer to an EVP_PKEY on success, or NULL on failure.
void* load_private_key(const char *username);

// Loads the public key associated with the given username from highscore/public_keys.
// Returns a pointer to an EVP_PKEY on success, or NULL on failure.
void* load_public_key(const char *username);

// Ensures a key pair exists for the given username.
// If not, a new key pair is generated and stored in the proper locations.
int ensure_keypair(const char *username);

#endif // ENCRYPTION_H

