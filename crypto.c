#include "crypto.h"
#include "file.h"
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
	randombytes_buf(metadata.salt, sizeof metadata.salt);
	randombytes_buf(metadata.nonce, sizeof metadata.nonce);

	if(error = derive_key(key, password_verify, &metadata, 1))
		return error;

	write_binary(&file_contents, HEADER, header_len);
	write_hex(&file_contents, memory_kbits, MEMORY_KBITS_HEX_LEN);
	write_hex(&file_contents, iterations, ITERATIONS_HEX_LEN);
	write_binary(&file_contents, metadata.salt, sizeof metadata.salt);
	write_binary(&file_contents, password_verify, sizeof password_verify);
	write_binary(&file_contents, encrypted_key, sizeof encrypted_key);
	write_binary(&file_contents, metadata.nonce, sizeof metadata.nonce);

	crypto_secretbox_easy(file_contents, file_contents, file_size, metadata.nonce, key);
	return SUCCESS;
}


error_type decrypt_file(unsigned char *file_contents, size_t file_size, const metadata_type *metadata) {
	error_type error;
	int password_correct = 0;
	unsigned char key[crypto_secretbox_KEYBYTES];

	while(!password_correct) {
		if(error = derive_key(key, metadata, 0))
			return error;

		if(crypto_secretbox_open_easy(file_contents, file_contents, file_size, metadata.nonce, key) == 0)
			password_correct = 1;
		else
			printf("Password is incorrect or the file is corrupted.\n");
	}

	return SUCCESS;
}


