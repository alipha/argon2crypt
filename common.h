#ifndef COMMON_H
#define COMMON_H

#include <sodium.h>

#define MIN_MEMORY_KBITS 1
#define MAX_MEMORY_KBITS 21
#define MIN_MEMORY_KBITS_STR "1 KiB"
#define MAX_MEMORY_KBITS_STR "2 GiB"
#define MIN_ITERATIONS 3
#define MAX_ITERATIONS 0x7FFFFFFFUL
#define PASSWORD_VERIFY_LEN 16
#define PASSWORD_MAX_SIZE 1024


typedef enum action_type {
	UNSPECIFIED = 0,
	ENCRYPT = 1,
	DECRYPT = 2
} action_type;


typedef enum error_type {
	SUCCESS = 0,
	LINE_LENGTH_EXCEEDED,
	INVALID_ARGUMENT,
	USER_CANCELLED,
	INVALID_SOURCE_FILE,
	INVALID_DEST_FILE,
	LIBSODIUM_INIT_ERROR,
	TERMINAL_ERROR,
	OUT_OF_MEMORY,
	ARGON2_ERROR,
	CORRUPT_SOURCE_FILE
} error_type;


typedef struct metadata_type {
	int memory_kbits;
	long iterations;
	unsigned char salt[crypto_pwhash_SALTBYTES];
	unsigned char password_verify[PASSWORD_VERIFY_LEN];
	unsigned char encrypted_key[crypto_secretbox_KEYBYTES];
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
} metadata_type;


int valid_memory_kbits(int memory_kbits);
int valid_iterations(int iterations);


#endif
