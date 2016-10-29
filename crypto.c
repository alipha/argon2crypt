#include "crypto.h"
#include "file.h"
#include "proxies.h"
#include "util.h"
#include <sodium.h>
#include <stdio.h>


error_type encrypt_file(unsigned char *file_contents, size_t file_size, int memory_kbits, long iterations) {
	error_type error;
	metadata_type metadata;
	char password[PASSWORD_MAX_SIZE];
	size_t header_len = strlen(HEADER);

	if((error = prompt_password(password, 1))) {
		sodium_memzero(password, sizeof password);
		return error;
	}

	metadata.memory_kbits = memory_kbits;
	metadata.iterations = iterations;
	p_randombytes_buf(metadata.salt, sizeof metadata.salt);
	p_randombytes_buf(metadata.nonce, sizeof metadata.nonce);
	p_randombytes_buf(metadata.encrypted_key, sizeof metadata.encrypted_key);

	if((error = derive_key(metadata.encrypted_key, metadata.password_verify, password, &metadata)))
		return error;

	file_contents = write_metadata(file_contents, &metadata);

	p_crypto_secretbox_easy(file_contents, file_contents, file_size, metadata.nonce, key);
	sodium_memzero(&metadata, sizeof metadata);
	return SUCCESS;
}


error_type decrypt_file(unsigned char *file_contents, size_t file_size, const metadata_type *metadata) {
	error_type error = SUCCESS;
	int password_correct = 0;
	char password[PASSWORD_MAX_SIZE];
	unsigned char key[sizeof metadata->encrypted_key];
	unsigned char password_verify[sizeof metadata->password_verify];

	while(!password_correct) {
		if((error = prompt_password(password, 0))) {
			break;

		memcpy(key, metadata->encrypted_key, sizeof key);

		if((error = derive_key(key, password_verify, password, metadata)))
			break;

		if(memcmp(password_verify, metadata->password_verify, sizeof password_verify) != 0) {
			p_fprintf(stderr, "Password is incorrect or the file is corrupted.\n");
		} else if(p_crypto_secretbox_open_easy(file_contents, file_contents, file_size, metadata.nonce, key) == 0) {
			password_correct = 1;
		} else {
			p_fprintf(stderr, "File is corrupted and unable to be decrypted.\n");
			error = CORRUPT_SOURCE_FILE;
			break;
		}
	}

	sodium_memzero(password, sizeof password);
	sodium_memzero(key, sizeof key);
	sodium_memzero(password_verify, sizeof password_verify);
	return error;
}


error_type derive_key(unsigned char *encrypted_key, unsigned char *password_verify, const char *password, const metadata_type *metadata) {
	unsigned char verify_and_xor_stream[sizeof metadata->password_verify + sizeof metadata->encrypted_key];
	unsigned char *p = verify_and_xor_stream;
	error_type result = SUCCESS;

	if(p_crypto_pwhash(p, sizeof verify_and_xor_stream, password, strlen(password), metadata->salt, metadata->iterations, 1 << (metadata->memory_kbits + 10), crypto_pwhash_ALG_DEFAULT) == 0) {
		read_binary(password_verify, &p, sizeof metadata->password_verify);
		xor_bytes(encrypted_key, p, sizeof metadata->encrypted_key);
	} else {
		p_fprintf(stderr, "The -m parameter is larger than the available memory. "); 

		if(confirm)
			p_fprintf(stderr, "Please reduce -m or free some memory.");
		else
			p_fprintf(stderr, "Please free some memory.");

		result = OUT_OF_MEMORY;
	}

	sodium_memzero(verify_and_xor_stream, sizeof verify_and_xor_stream);
	return result;
}



