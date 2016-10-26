#ifndef CRYPTO_H
#define CRYPTO_H

#include "common.h"
#include <sodium.h>
#include <stdint.h>


typedef struct metadata_type {
	int memory_kbits;
	long iterations;
	unsigned char salt[crypto_pwhash_SALTBYTES];
	unsigned char password_verify[PASSWORD_VERIFY_LEN];
	unsigned char encrypted_key[crypto_secretbox_KEYBYTES];
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
} metadata_type;


error_type encrypt_file(unsigned char *file_contents, size_t file_size, int memory_kbits, long iterations);
error_type decrypt_file(unsigned char *file_contents, size_t file_size, const metadata_type *metadata);


#endif
