#include "crypto.h"
#include "file.h"
#include "proxies.h"
#include "util.h"
#include <sodium.h>
#include <stdio.h>


error_type encrypt_file(unsigned char *file_contents, size_t file_size, int memory_kbits, long iterations) {
	error_type error;
	metadata_type metadata;
	unsigned char key[crypto_secretbox_KEYBYTES];
	unsigned char password_verify[PASSWORD_VERIFY_LEN];
	size_t header_len = strlen(HEADER);

	metadata.memory_kbits = memory_kbits;
	metadata.iterations = iterations;
	p_randombytes_buf(metadata.salt, sizeof metadata.salt);
	p_randombytes_buf(metadata.nonce, sizeof metadata.nonce);

	if(error = derive_key(key, password_verify, &metadata, 1))
		return error;

	write_binary(&file_contents, HEADER, header_len);
	write_hex(&file_contents, memory_kbits, MEMORY_KBITS_HEX_LEN);
	write_hex(&file_contents, iterations, ITERATIONS_HEX_LEN);
	write_binary(&file_contents, metadata.salt, sizeof metadata.salt);
	write_binary(&file_contents, password_verify, sizeof password_verify);
	write_binary(&file_contents, encrypted_key, sizeof encrypted_key);
	write_binary(&file_contents, metadata.nonce, sizeof metadata.nonce);

	p_crypto_secretbox_easy(file_contents, file_contents, file_size, metadata.nonce, key);
	return SUCCESS;
}


error_type decrypt_file(unsigned char *file_contents, size_t file_size, const metadata_type *metadata) {
	error_type error;
	int password_correct = 0;
	unsigned char key[crypto_secretbox_KEYBYTES];

	while(!password_correct) {
		if(error = derive_key(key, metadata, 0))
			return error;

		if(p_crypto_secretbox_open_easy(file_contents, file_contents, file_size, metadata.nonce, key) == 0)
			password_correct = 1;
		else
			p_fprintf(stderr, "Password is incorrect or the file is corrupted.\n");
	}

	return SUCCESS;
}


error_type derive_key(unsigned char *key, unsigned char *password_verify, const char *password, const metadata_type *metadata) {
	unsigned char verify_and_xor_stream[sizeof metadata->password_verify + sizeof metadata->encrypted_key];
	unsigned char *p = verify_and_xor_stream;
	error_type result = SUCCESS;

	if(p_crypto_pwhash(p, sizeof verify_and_xor_stream, password, strlen(password), metadata->salt, metadata->iterations, 1 << (metadata->memory_kbits + 10), crypto_pwhash_ALG_DEFAULT) == 0) {
		read_binary(password_verify, &p, sizeof metadata->password_verify);
		read_binary(key, &p, sizeof metadata->encrypted_key);
	} else {
		p_fprintf(stderr, "The -m parameter is larger than the available memory. "); 

		if(confirm)
			p_fprintf(stderr, "Please reduce -m or free some memory.");
		else
			p_fprintf(stderr, "Please free some memory.");

		result = OUT_OF_MEMORY;
	}

	return result;
}



