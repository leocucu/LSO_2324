#include "passwordhash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>

void hash_password(const char *password, char *hashed_password) {
    if (sodium_init() == -1) {
        fprintf(stderr, "sodium_init failed\n");
        exit(1);
    }

    if (crypto_pwhash_str(hashed_password, password, strlen(password),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
}

int verify_password(const char *password, const char *hashed_password) {
    if (crypto_pwhash_str_verify(hashed_password, password, strlen(password)) != 0) {
        return 1; // Password non valida
    }
    return 0; // Password valida
}