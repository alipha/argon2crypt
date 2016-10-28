#include "common.h"
#include "crypto.h"
#include "file.h"
#include "proxies.h"
#include <stdio.h>


int main(int argc, char **argv) {
	const char *source_filename;
	const char *dest_filename;
	unsigned char *file_contents;
	size_t file_size;
	int memory_kbits = 14;
	long iterations = 3;
	action_type action = UNSPECIFIED;
	int i;
	error_type error;

	p_fprintf(stdout, "\n");

	if(sodium_init() == -1) {
		p_fprintf(stderr, "Unable to initialize libsodium encryption library\n\n");
		return LIBSODIUM_INIT_ERROR;
	}

	if(argc < 4) {
		p_fprintf(stderr, "Not enough arguments.\n");
		return usage(argv[0]);
	}

	source_filename = argv[argc - 2];
	dest_filename = argv[argc - 1];

	if((error = parse_args(argv, argc, &action, &memory_kbits, &iterations)))
		return error;

	if((error = read_file(source_filename, &file_contents, &file_size, action)))
		return error;
	
	if(action == ENCRYPT) {
		error = encrypt_file(file_contents, file_size, memory_kbits, iterations);
		file_size += FULL_HEADER_LEN;
	} else {
		if((error = extract_metadata(&metadata, argv[0], source_filename, &file_contents, file_size)))
			return error;

		error = decrypt_file(file_contents, &metadata);
		file_size -= FULL_HEADER_LEN;
	}

	if(error)
		return error;

	return write_file(dest_filename, file_contents, file_size);
}



