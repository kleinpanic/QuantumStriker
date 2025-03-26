#include "encryption.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <errno.h>

#ifndef USERNAME_FILE
#define USERNAME_FILE ".username"
#endif

// Computes SHA-256 over input data and outputs a hex string.
void hash_score(const char *data, char *output_hash) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)data, strlen(data), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output_hash + (i * 2), "%02x", hash[i]);
    }
    output_hash[SHA256_DIGEST_LENGTH * 2] = '\0';
}

// Helper: Write the private key to the .username file.
// Format: first line is the username, then the PEM-encoded private key.
static int write_private_key_to_username(const char *username, EVP_PKEY *pkey) {
    FILE *fp = fopen(USERNAME_FILE, "w");
    if (!fp) {
        DEBUG_PRINT(1, 0, "Error opening %s for writing: %s", USERNAME_FILE, strerror(errno));
        return 0;
    }
    // Write the username on the first line.
    fprintf(fp, "%s\n", username);
    
    BIO *bio = BIO_new(BIO_s_mem());
    if (!bio) {
        fclose(fp);
        return 0;
    }
    if (!PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL)) {
        DEBUG_PRINT(1, 0, "Error writing private key to memory BIO");
        BIO_free(bio);
        fclose(fp);
        return 0;
    }
    char *pem_data = NULL;
    long pem_len = BIO_get_mem_data(bio, &pem_data);
    if (pem_len <= 0) {
        BIO_free(bio);
        fclose(fp);
        return 0;
    }
    fwrite(pem_data, 1, pem_len, fp);
    BIO_free(bio);
    fclose(fp);
    return 1;
}

// Helper: Write the public key to highscore/public_keys/<username>_public.pem.
// Ensures that the "highscore" and "highscore/public_keys" directories exist.
static int write_public_key(const char *username, EVP_PKEY *pkey) {
    const char *pub_dir = "highscore/public_keys";
    struct stat st;
    if (stat("highscore", &st) != 0) {
        if (mkdir("highscore", 0755) != 0) {
            DEBUG_PRINT(1, 0, "Error creating highscore directory: %s", strerror(errno));
            return 0;
        }
    }
    if (stat(pub_dir, &st) != 0) {
        if (mkdir(pub_dir, 0755) != 0) {
            DEBUG_PRINT(1, 0, "Error Creating public keys directory %s: %s", pub_dir, strerror(errno));
            return 0;
        }
    }
    char pub_filename[256];
    snprintf(pub_filename, sizeof(pub_filename), "%s/%s_public.pem", pub_dir, username);
    FILE *fp = fopen(pub_filename, "wb");
    if (!fp) {
        DEBUG_PRINT(1, 0, "Error opening %s for writing: %s", pub_filename, strerror(errno));
        return 0;
    }
    if (!PEM_write_PUBKEY(fp, pkey)) {
        DEBUG_PRINT(1, 0, "Error Writing public key for user %s", username);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

// Generates a new RSA key pair for the given username.
// The private key is stored in the .username file and the public key in highscore/public_keys.
int generate_keypair(const char *username) {
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        DEBUG_PRINT(2, 0, "Error creating EVP_PKEY_CTX for user %s", username);
        return 0;
    }
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        DEBUG_PRINT(2, 0, "Error initializing key generation for user %s", username);
        goto cleanup;
    }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        DEBUG_PRINT(2, 0, "Error setting RSA key size for user %s", username);
        goto cleanup;
    }
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        DEBUG_PRINT(2, 0, "Error during key generation for user %s", username);
        goto cleanup;
    }
    
    if (!write_private_key_to_username(username, pkey)) {
        DEBUG_PRINT(2, 0, "Error saving private key for user %s in %s", username, USERNAME_FILE);
        goto cleanup;
    }
    if (!write_public_key(username, pkey)) {
        DEBUG_PRINT(2, 0, "Error saving public key for user %s", username);
        goto cleanup;
    }
    
    DEBUG_PRINT(2, 3, "Key pair generated for user %s", username);
    ret = 1; // success

cleanup:
    if (pkey) EVP_PKEY_free(pkey);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return ret;
}

// Loads the private key from the .username file.
// Expects the file to have the username on the first line, followed by the PEM block.
void* load_private_key(const char *username) {
    // Ensure a valid key pair exists.
    if (!ensure_keypair(username)) {
        DEBUG_PRINT(2, 0, "Failed to ensure key pair for %s", username);
        return NULL;
    }
    FILE *fp = fopen(USERNAME_FILE, "r");
    if (!fp) {
        DEBUG_PRINT(2, 0, "Could not open %s for reading private key for user %s", USERNAME_FILE, username);
        return NULL;
    }
    char buffer[256];
    if (!fgets(buffer, sizeof(buffer), fp)) {
        fclose(fp);
        return NULL;
    }
    // Read the entire file into memory.
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *file_contents = malloc(fsize + 1);
    if (!file_contents) {
        fclose(fp);
        return NULL;
    }
    fread(file_contents, 1, fsize, fp);
    file_contents[fsize] = '\0';
    fclose(fp);
    
    // Look for the PEM header.
    char *pem_start = strstr(file_contents, "-----BEGIN");
    if (!pem_start) {
        DEBUG_PRINT(2, 0, "Private key PEM block not found in %s", USERNAME_FILE);
        free(file_contents);
        // If no PEM block, regenerate keys.
        if (generate_keypair(username)) {
            return load_private_key(username);
        }
        return NULL;
    }
    BIO *bio = BIO_new_mem_buf(pem_start, -1);
    if (!bio) {
        free(file_contents);
        return NULL;
    }
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    if (!pkey) {
        DEBUG_PRINT(2, 0, "Error reading private key from memory buffer for user %s", username);
    }
    BIO_free(bio);
    free(file_contents);
    return pkey;
}

// Loads the public key for the given username from highscore/public_keys/<username>_public.pem.
void* load_public_key(const char *username) {
    char pub_filename[256];
    snprintf(pub_filename, sizeof(pub_filename), "highscore/public_keys/%s_public.pem", username);
    FILE *fp = fopen(pub_filename, "rb");
    if (!fp) {
        DEBUG_PRINT(2, 0, "Public key file %s not found for user %s", pub_filename, username);
        return NULL;
    }
    EVP_PKEY *pkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);
    if (!pkey) {
        DEBUG_PRINT(2, 0, "Error loading public key from %s for user %s", pub_filename, username);
    }
    return pkey;
}

// Ensures a key pair exists for the given username.
// If the .username file does not exist or lacks a valid private key, a new key pair is generated.
int ensure_keypair(const char *username) {
    FILE *fp = fopen(USERNAME_FILE, "r");
    if (!fp) {
        DEBUG_PRINT(2, 1, "Private key file %s not found; generating new key pair for %s.", USERNAME_FILE, username);
        return generate_keypair(username);
    }
    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "-----BEGIN") != NULL) {
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (!found) {
        DEBUG_PRINT(2, 1, "No private key PEM block found in %s; generating new key pair for %s.", USERNAME_FILE, username);
        return generate_keypair(username);
    }
    DEBUG_PRINT(2, 3, "Private key exists for user %s in %s.", username, USERNAME_FILE);
    return 1;
}
