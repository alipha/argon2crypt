#ifndef COMMON_H
#define COMMON_H

#define MIN_MEMORY_KBITS 1
#define MAX_MEMORY_KBITS 21
#define MIN_MEMORY_KBITS_STR "1 KiB"
#define MAX_MEMORY_KBITS_STR "2 GiB"
#define MIN_ITERATIONS 3
#define MAX_ITERATIONS 0x7FFFFFFFL


typedef enum action_type {
	UNSPECIFIED = 0,
	ENCRYPT = 1,
	DECRYPT = 2
} action_type;


typedef enum error_type {
	SUCCESS = 0,
	INVALID_ARGUMENT,
	USER_CANCELLED,
	INVALID_SOURCE_FILE,
	INVALID_DEST_FILE,
	LIBSODIUM_INIT_ERROR,
	OUT_OF_MEMORY,
	ARGON2_ERROR,
	CORRUPT_DEST_FILE
} error_type;


int valid_memory_kbits(int memory_kbits);
int valid_iterations(int iterations);


#endif
