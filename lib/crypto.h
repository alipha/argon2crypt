#ifndef CRYPTO_H
#define CRYPTO_H

#include "common.h"
#include <sodium.h>
#include <stddef.h>


error_type encrypt_file(unsigned char *file_contents, size_t file_size, int memory_kbits, long iterations);
error_type decrypt_file(unsigned char *file_contents, size_t file_size, const metadata_type *metadata);
error_type derive_key(unsigned char *encrypted_key, unsigned char *password_verify, const char *password, const metadata_type *metadata, int confirm);


#endif
