#include "file.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


error_type read_file(const char *filename, unsigned char **source_file, size_t *file_size, action_type action) {
	error_type error;
	error_type error2;
	long size;
	size_t read;
	int header_len = 0;
	FILE *file = fopen(filename, "rb");

	if(!file) {
		printf("Unable to open source file: %s\n", filename);
		perror(0);
		return INVALID_SOURCE_FILE;
	}

	error = fseek(file, 0L, SEEK_END);
	size = ftell(file);
	error2 = fseek(file, 0L, SEEK_SET);

	if(error || error2 || size == -1) {
		printf("Failure to read source file: %s\n", filename);
		perror(0);
		fclose(file);
		return INVALID_SOURCE_FILE;
	}

	if(action == ENCRYPT) 
		header_len = FULL_HEADER_LEN;

	*source_file = malloc(size + header_len);
	
	if(!*source_file) {
		printf("Not enough memory available to load source file \"%s\" into memory. Memory required: %ld MiB\n", filename, size / (1L << 20));
		fclose(file);
		return OUT_OF_MEMORY;
	}

	read = fread(*source_file + header_len, 1, size, file);

	if(read != size) {
		printf("Failure to read source file: %s\n", filename);
		perror(0);
		fclose(file);
		return INVALID_SOURCE_FILE;
	}

	*file_size = size;
	fclose(file);
	return SUCCESS;
}


error_type write_file(const char *filename, const unsigned char *file_contents, size_t file_size) {
	size_t written;
	FILE *file = fopen(filename, "wb");

	if(!file) {
		printf("Unable to open dest file: %s\n", filename);
		perror(0);
		return INVALID_DEST_FILE;
	}

	written = fwrite(file_contents, 1, file_size, file);

	if(written != file_size) {
		printf("Failure to write to dest file: %s\n", filename);
		perror(0);
		fclose(file);
		return INVALID_DEST_fILE;
	}

	fclose(file);
	return SUCCESS;
}


error_type extract_metadata(metadata_type *metadata, const char *exe, const char *filename, const unsigned char **file_contents, size_t file_size) {
	error_type error;
	size_t header_len;
	const unsigned char *p;

	header_len = strlen(HEADER); 
	error = (file_size < FULL_HEADER_LEN);

	if(!error) {
		p = *file_contents + header_len;
		metadata->memory_kbits = read_hex(&p, MEMORY_KBITS_HEX_LEN);
		metadata->iterations = read_hex(&p, ITERATIONS_HEX_LEN);
		read_binary(metadata->salt, &p, sizeof metadata->salt);
		read_binary(metadata->password_verify, &p, sizeof metadata->password_verify);
		read_binary(metadata->encrypted_key, &p, sizeof metadata->encrypted_key);
		read_binary(metadata->nonce, &p, sizeof metadata->nonce);
	}
	
	if(error || memcmp(*file_contents, HEADER, header_len) != 0 || !valid_memory_kbits(metadata->memory_kbits) || !valid_iterations(metadata->iterations)) {
		printf("The dest file \"%s\" does not appear to be encrypted with %s or has been corrupted.\n", filename, exe);
		return CORRUPT_DEST_FILE;
	}

	*file_contents = p;
	return SUCCESS;
}

