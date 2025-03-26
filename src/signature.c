#include "signature.h"
#include "encryption.h"
#include "debug.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <stdio.h>

// Helper: create a string representation of the block data to sign (excluding proof, signature, nonce).
static void get_block_data_string(const ScoreBlock *block, char *buffer, size_t bufsize) {
    snprintf(buffer, bufsize, "%s|%d|%ld|%s|%u",
             block->username,
             block->score,
             block->timestamp,
             block->prev_hash,
             block->nonce);
}

int sign_score(const ScoreBlock *block, const char *username, char *signature_hex) {
    int ret = 0;
    EVP_PKEY *pkey = load_private_key(username);
    if (!pkey) {
        DEBUG_PRINT(2, 0, "Failed to load private key for %s", username);
        return 0;
    }
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        DEBUG_PRINT(2, 0, "Failed to allocate EVP_MD_CTX for user %s", username);
        EVP_PKEY_free(pkey);
        return 0;
    }
    if (EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, pkey) != 1) {
        DEBUG_PRINT(2, 0, "EVP_DigestSignInit failed for user %s", username);
        goto cleanup;
    }

    char data[512];
    get_block_data_string(block, data, sizeof(data));
    if (EVP_DigestSignUpdate(mdctx, data, strlen(data)) != 1) {
        DEBUG_PRINT(2, 0, "EVP_DigestSignUpdate failed for user %s", username);
        goto cleanup;
    }

    size_t sig_len = 0;
    if (EVP_DigestSignFinal(mdctx, NULL, &sig_len) != 1) {
        DEBUG_PRINT(2, 0, "EVP_DigestSignFinal (first call) failed for user %s", username);
        goto cleanup;
    }
    unsigned char *sig = (unsigned char*)malloc(sig_len);
    if (!sig) {
        DEBUG_PRINT(2, 0, "Memory allocation for signature failed for user %s", username);
        goto cleanup;
    }
    if (EVP_DigestSignFinal(mdctx, sig, &sig_len) != 1) {
        DEBUG_PRINT(2, 0, "EVP_DigestSignFinal (second call) failed for user %s", username);
        free(sig);
        goto cleanup;
    }

    // Convert binary signature to hex string.
    for (size_t i = 0; i < sig_len; i++) {
        sprintf(signature_hex + (i * 2), "%02x", sig[i]);
    }
    signature_hex[sig_len * 2] = '\0';
    free(sig);
    ret = 1;
cleanup:
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    return ret;
}

int verify_score_signature(const ScoreBlock *block, const char *username, const char *signature_hex) {
    int ret = 0;
    EVP_PKEY *pkey = load_public_key(username);
    if (!pkey) {
        DEBUG_PRINT(2, 0, "Failed to load public key for %s", username);
        return 0;
    }
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        DEBUG_PRINT(2, 0, "Failed to allocate EVP_MD_CTX for verification for user %s", username);
        EVP_PKEY_free(pkey);
        return 0;
    }
    if (EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pkey) != 1) {
        DEBUG_PRINT(2, 0, "EVP_DigestVerifyInit failed for user %s", username);
        goto cleanup;
    }

    char data[512];
    get_block_data_string(block, data, sizeof(data));
    if (EVP_DigestVerifyUpdate(mdctx, data, strlen(data)) != 1) {
        DEBUG_PRINT(2, 0, "EVP_DigestVerifyUpdate failed for user %s", username);
        goto cleanup;
    }

    // Convert hex signature back to binary.
    size_t sig_len = strlen(signature_hex) / 2;
    unsigned char *sig = (unsigned char*)malloc(sig_len);
    if (!sig) {
        DEBUG_PRINT(2, 0, "Memory allocation for signature verification failed for user %s", username);
        goto cleanup;
    }
    for (size_t i = 0; i < sig_len; i++) {
        sscanf(signature_hex + 2*i, "%2hhx", &sig[i]);
    }
    ret = (EVP_DigestVerifyFinal(mdctx, sig, sig_len) == 1);
    if (!ret) {
        DEBUG_PRINT(2, 0, "Signature verification failed for user %s", username);
    }
    free(sig);
cleanup:
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    return ret;
}

