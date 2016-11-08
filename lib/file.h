#ifndef FILE_H
#define FILE_H

#include "common.h"
#include <stddef.h>
#include <string.h>


#define HEADER "argon2crypt01"
#define HEADER_LEN strlen(HEADER)
#define MEMORY_KBITS_HEX_LEN 2
#define ITERATIONS_HEX_LEN 8
#define FULL_HEADER_LEN (HEADER_LEN + MEMORY_KBITS_HEX_LEN + ITERATIONS_HEX_LEN + crypto_pwhash_SALTBYTES + PASSWORD_VERIFY_LEN + crypto_secretbox_KEYBYTES + crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES)


error_type read_file(const char *filename, unsigned char **source_file, size_t *file_size, action_type action);
error_type write_file(const char *filename, const unsigned char *file_contents, size_t file_size);

error_type extract_metadata(metadata_type *metadata, const char *exe, const char *filename, const unsigned char **file_contents, size_t file_size);
unsigned char *write_metadata(unsigned char *file_contents, const metadata_type *metadata);

#endif
