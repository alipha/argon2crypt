#include "common.h"
#include "crypto.h"
#include "file.h"
#include "proxies.h"
#include <stdio.h>


error_type run(int argc, char **argv, unsigned char **file_contents, size_t *file_size, metadata_type *metadata);


int main(int argc, char **argv) {
	error_type error;
	unsigned char *file_contents = 0;
	size_t file_size = 0;
	metadata_type metadata;

	error = run(argc, argv, &file_contents, &file_size, &metadata);

	sodium_memzero(file_contents, file_size);
	sodium_memzero(&metadata, sizeof metadata);
	p_free(file_contents);
	return (int)error;
}


error_type run(int argc, char **argv, unsigned char **file_contents, size_t *file_size, metadata_type *metadata) {
	unsigned char *p;
	const char *source_filename;
	const char *dest_filename;
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

	if((error = read_file(source_filename, file_contents, file_size, action)))
		return error;
	
	p = *file_contents;

	if(action == ENCRYPT) {
		error = encrypt_file(*p, *file_size, memory_kbits, iterations);
		file_size += FULL_HEADER_LEN;
	} else {
		if((error = extract_metadata(metadata, argv[0], source_filename, &p, file_size)))
			return error;

		error = decrypt_file(p, *file_size, metadata);
		file_size -= FULL_HEADER_LEN;
	}

	if(error)
		return error;

	return write_file(dest_filename, p, *file_size);
}



