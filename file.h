#ifndef FILE_H
#define FILE_H

#include "common.h"
#include <stdint.h>


#define HEADER "argon2crypt01"
#define MEMORY_KBITS_HEX_LEN 2
#define ITERATIONS_HEX_LEN 8
#define PASSWORD_VERIFY_LEN 16
#define FULL_HEADER_LEN (strlen(HEADER) + MEMORY_KBITS_HEX_LEN + ITERATIONS_HEX_LEN + crypto_pwhash_SALTBYTES + PASSWORD_VERIFY_LEN + crypto_secretbox_KEYBYTES + crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES)


error_type read_file(const char *filename, unsigned char **source_file, size_t *file_size, action_type action);
error_type write_file(const char *filename, const unsigned char *file_contents, size_t file_size);

error_type extract_metadata(metadata_type *metadata, const char *exe, const char *filename, const unsigned char **file_contents, size_t file_size);


#endif
