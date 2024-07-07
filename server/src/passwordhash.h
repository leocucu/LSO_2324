#ifndef PASSWORDHASH_H
#define PASSWORDHASH_H

#include <sodium.h>

#define HASH_SIZE crypto_pwhash_STRBYTES

void hash_password(const char *password, char *hashed_password);
int verify_password(const char *password, const char *hashed_password);

#endif