#include "file.h"
#include "common.h"
#include "proxies.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


error_type read_file(const char *filename, unsigned char **source_file, size_t *file_size, action_type action) {
	error_type error;
	error_type error2;
	long size;
	size_t read;
	int header_len = 0;
	FILE *file = p_fopen(filename, "rb");

	if(!file) {
		p_fprintf(stderr, "Unable to open source file: %s\n", filename);
		p_perror(0);
		return INVALID_SOURCE_FILE;
	}

	error = p_fseek(file, 0L, SEEK_END);
	size = p_ftell(file);
	error2 = p_fseek(file, 0L, SEEK_SET);

	if(error || error2 || size == -1) {
		p_fprintf(stderr, "Failure to read source file: %s\n", filename);
		p_perror(0);
		p_fclose(file);
		return INVALID_SOURCE_FILE;
	}

	if(action == ENCRYPT) 
		header_len = FULL_HEADER_LEN;

	*source_file = p_malloc(size + header_len);
	
	if(!*source_file) {
		p_fprintf(stderr, "Not enough memory available to load source file \"%s\" into memory. Memory required: %ld MiB\n", filename, size / (1L << 20));
		p_fclose(file);
		return OUT_OF_MEMORY;
	}

	read = p_fread(*source_file + header_len, 1, size, file);

	if(read != (unsigned long)size) {
		p_fprintf(stderr, "Failure to read source file: %s\n", filename);
		p_perror(0);
		p_fclose(file);
		return INVALID_SOURCE_FILE;
	}

	*file_size = size;
	p_fclose(file);
	return SUCCESS;
}


error_type write_file(const char *filename, const unsigned char *file_contents, size_t file_size) {
	size_t written;
	FILE *file = p_fopen(filename, "wb");

	if(!file) {
		p_fprintf(stderr, "Unable to open dest file: %s\n", filename);
		p_perror(0);
		return INVALID_DEST_FILE;
	}

	written = p_fwrite(file_contents, 1, file_size, file);

	if(written != file_size) {
		p_fprintf(stderr, "Failure to write to dest file: %s\n", filename);
		p_perror(0);
		p_fclose(file);
		return INVALID_DEST_FILE;
	}

	p_fclose(file);
	return SUCCESS;
}


error_type extract_metadata(metadata_type *metadata, const char *exe, const char *filename, const unsigned char **file_contents, size_t file_size) {
	error_type error;
	const unsigned char *p;

	error = (file_size < FULL_HEADER_LEN);

	if(!error) {
		p = *file_contents + HEADER_LEN;
		metadata->memory_kbits = read_hex(&p, MEMORY_KBITS_HEX_LEN);
		metadata->iterations = read_hex(&p, ITERATIONS_HEX_LEN);
		read_binary(metadata->salt, &p, sizeof metadata->salt);
		read_binary(metadata->password_verify, &p, sizeof metadata->password_verify);
		read_binary(metadata->encrypted_key, &p, sizeof metadata->encrypted_key);
		read_binary(metadata->nonce, &p, sizeof metadata->nonce);
	}
	
	if(error || memcmp(*file_contents, HEADER, HEADER_LEN) != 0 || !valid_memory_kbits(metadata->memory_kbits) || !valid_iterations(metadata->iterations)) {
		p_fprintf(stderr, "The source file \"%s\" does not appear to be encrypted with %s or has been corrupted.\n", filename, exe);
		return CORRUPT_SOURCE_FILE;
	}

	*file_contents = p;
	return SUCCESS;
}


unsigned char *write_metadata(unsigned char *file_contents, const metadata_type *metadata) {

	write_binary(&file_contents, (unsigned char *)HEADER, HEADER_LEN);
	write_hex(&file_contents, metadata->memory_kbits, MEMORY_KBITS_HEX_LEN);
	write_hex(&file_contents, metadata->iterations, ITERATIONS_HEX_LEN);
	write_binary(&file_contents, metadata->salt, sizeof metadata->salt);
	write_binary(&file_contents, metadata->password_verify, sizeof metadata->password_verify);
	write_binary(&file_contents, metadata->encrypted_key, sizeof metadata->encrypted_key);
	write_binary(&file_contents, metadata->nonce, sizeof metadata->nonce);

	return file_contents;
}

