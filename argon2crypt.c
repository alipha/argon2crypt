#include <sodium.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

#define HEADER "argon2crypt01"
#define MEMORY_KBITS_HEX_LEN 2
#define ITERATIONS_HEX_LEN 8
#define PASSWORD_VERIFY_LEN 16
#define FULL_HEADER_LEN (strlen(HEADER) + MEMORY_KBITS_HEX_LEN + ITERATIONS_HEX_LEN + crypto_pwhash_SALTBYTES + PASSWORD_VERIFY_LEN + crypto_secretbox_KEYBYTES + crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES)
#define MIN_MEMORY_KBITS 1
#define MAX_MEMORY_KBITS 21
#define MIN_MEMORY_KBITS_STR "1 KiB"
#define MAX_MEMORY_KBITS_STR "2 GiB"
#define MIN_ITERATIONS 3
#define MAX_ITERATIONS 0x7FFFFFFFL
#define EXCESSIVE_COST (1UL << 26)	/* "several minutes" */


typedef enum error_type {
	SUCCESS = 0,
	INVALID_ARGUMENT,
	USER_CANCELLED,
	INVALID_SOURCE_FILE,
	INVALID_DEST_FILE,
	LIBSODIUM_INIT_ERROR,
	OUT_OF_MEMORY,
	ARGON2_ERROR,
	CORRUPT_DEST_FILE
} error_type;


typedef enum action_type {
	UNSPECIFIED = 0,
	ENCRYPT = 1,
	DECRYPT = 2
} action_type;


typedef struct metadata_type {
	int memory_kbits;
	long iterations;
	unsigned char salt[crypto_pwhash_SALTBYTES];
	unsigned char password_verify[PASSWORD_VERIFY_LEN];
	unsigned char encrypted_key[crypto_secretbox_KEYBYTES];
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
} metadata_type;


int parse_args(char **argv, int argc, action_type *action, int *memory_kbits, long *iterations);
int usage(const char *exe);

int read_file(const char *filename, unsigned char **source_file, size_t *file_size, action_type action);
int write_file(const char *filename, const unsigned char *file_contents, size_t file_size);
int encrypt_file(unsigned char *file_contents, size_t file_size, int memory_kbits, long iterations);
int decrypt_file(unsigned char *file_contents, size_t file_size, const metadata_type *metadata);

int extract_metadata(metadata_type *metadata, const char *exe, const char *filename, const unsigned char **file_contents, size_t file_size);

void read_binary(unsigned char *dest, const unsigned char **src, size_t amount);
void write_binary(unsigned char **dest, const unsigned char *src, size_t amount);
long read_hex(const unsigned char **str, size_t digits);
void write_hex(unsigned char **dest, long value, size_t digits);

void xor_bytes(unsigned char *dest, unsigned char *src, size_t amount);

int derive_key(unsigned char *key, unsigned char *password_verify, const metadata_type *metadata, int confirm);
size_t read_password(char **lineptr, size_t *n, FILE *stream);
int check_excessive(action_type action, int memory_kbits, long iterations);

int valid_memory_kbits(int memory_kbits);
int valid_iterations(int iterations);



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

	printf("\n");

	if(sodium_init() == -1) {
		printf("Unable to initialize libsodium encryption library\n\n");
		return LIBSODIUM_INIT_ERROR;
	}

	if(argc < 4) {
		printf("Not enough arguments.\n");
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


int parse_args(char **argv, int argc, action_type *action, int *memory_kbits, long *iterations) {
	int i;
	const char *arg;

	for(i = 1; i < argc - 2; i++) {
		arg = argv[i];

		if(strcmp(arg, "-e") == 0) {
			*action = ENCRYPT;
		} else if(strcmp(arg, "-d") == 0) {
			*action = DECRYPT;
		} else if(strcmp(arg, "-m") == 0) {
			if(i == argc - 3) {
				printf("Not enough arguments.\n");
				return usage(argv[0]);
			}
			*memory_kbits = strtoul(argv[++i], 0, 10);
			if(valid_memory_kbits(*memory_kbits)) {
				printf("-m must be between %d and %d. (%s and %s of memory)\n", MIN_MEMORY_KBITS, MAX_MEMORY_KBITS, MIN_MEMORY_KBITS_STR, MAX_MEMORY_KBITS_STR);
				printf("Valid values of -m may be further constrained by\n");
				printf("the amount of memory on your system.\n");
				return usage(argv[0]);
			}
		} else if(strcmp(arg, "-t") == 0) {
			if(i == argc - 3) {
				printf("Not enough arguments.\n");
				return usage(argv[0]);
			}
			*iterations = strtoul(argv[++i], 0, 10);
			if(valid_iterations(*iterations)) {
				printf("-t must be at least %d and less than %d.\n", MIN_ITERATIONS, MAX_ITERATIONS);
				return usage(argv[0]);
			}
		} else {
			printf("Unknown argument: %s\n", arg);
			return usage(argv[0]);
		}
	}

	if(*action == UNSPECIFIED) {
		printf("You must specify either -e or -d for encrypting or decrypting.\n");
		return usage(argv[0]);
	}

	return SUCCESS;
}


int usage(const char *exe) {
	printf("\n");
	printf("Usage: %s [-e|-d] [-m N] [-t N] source dest\n", exe);
	printf("\n");
	printf("Encrypts or decrypts a file with a key derived from the user-entered passphrase\n");
	printf("using Argon2 and ChaCha20-Poly1305.\n");
	printf("\n");
	printf("source, dest, and -e or -d are required.\n");
	printf("\n");
	printf("\t-e\tEncrypts file 'source' and saves the encrypted result as 'dest'\n");
	printf("\t-d\tDecrypts file 'source' and saves the decrypted result as 'source'\n");
	printf("\t-m N\tSets the memory usage to 2^N KiB. (Default is 14 (16 MiB))\n");
	printf("\t-t N\tSets the number of iterations to N. (Default is 3)\n");
	printf("\n");
	printf("-m and -t are ignored when decrypting. The values used to encrypt the file are used.");
	return INVALID_ARGUMENT;
}


int read_file(const char *filename, unsigned char **source_file, size_t *file_size, action_type action) {
	int error;
	int error2;
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


int write_file(const char *filename, const unsigned char *file_contents, size_t file_size) {
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


int encrypt_file(unsigned char *file_contents, size_t file_size, int memory_kbits, long iterations) {
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


int decrypt_file(unsigned char *file_contents, size_t file_size, const metadata_type *metadata) {
	int error;
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


int extract_metadata(metadata_type *metadata, const char *exe, const char *filename, const unsigned char **file_contents, size_t file_size) {
	int error;
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
	return 0;
}


void read_binary(unsigned char *dest, const unsigned char **src, size_t amount) {
	memcpy(dest, *src, amount);
	*src += amount;
}


void write_binary(unsigned char **dest, const unsigned char *src, size_t amount) {
	memcpy(*dest, src, amount);
	*dest += amount;
}


long read_hex(const unsigned char **str, size_t digits) {
	char ch;
	long value = 0;
	int i;
	unsigned char *p = *str;

	for(i = 0; i < digits; i++) {
		if(value > LONG_MAX >> 4)
			return -1;

		ch = p[i];
		value <<= 4;

		if(ch >= '0' && ch <= '9')
			value += ch - '0';
		else if(ch >= 'a' && ch <= 'f')
			value += ch - 'a' + 10;
		else if(ch >= 'A' && ch <= 'F')
			value += ch - 'A' + 10;
		else
			return -1;
	}

	*str = p;
}


void write_hex(unsigned char **dest, long value, size_t digits) {
	int i;
	long digit;
	unsigned char *p = *dest;

	p += digits;
	*dest = p;

	for(i = 0; i < digits; i++) {
		p--;
		digit = (value & 0xf);
		value >>= 4;

		if(digit >= 0 && digit <= 9)
			*p = '0' + digit;
		else
			*p = 'a' + (digit - 10);
	}
}


void xor_bytes(unsigned char *dest, unsigned char *src, size_t amount) {
	for(size_t i = 0; i < amount; i++)
		dest[i] ^= src[i];
}


// TODO: crypto_pwhash needs to generate both the password_verify and key
int derive_key(unsigned char *key, unsigned char *password_verify, const metadata_type *metadata, int confirm) {
	unsigned char verify_and_key[sizeof metadata->password_verify + sizeof metadata->encrypted_key];
	unsigned char *p = verify_and_key;
	int result = 0;
	char *password = 0;
	size_t password_len = 0;
	char *confirm_password = 0;
	size_t confirm_len = 0;

	do {
		printf("Enter passphrase: ");

		if(read_password(&password, &password_len, stdin) == -1 || password[0] == '\0') {
			result = USER_CANCELLED;
			break;
		}

		if(!confirm)
			break;

		printf("Confirm passphrase: ");

		if(read_password(&confirm_password, &confirm_len, stdin) == -1) {
			result = USER_CANCELLED;
			break;
		}
		
		if(strcmp(password, confirm_password) == 0)
			confirm = 0;
		else
			printf("Passphrases do not match.\n");
	} while(confirm);


	if(result == 0) {
		if(crypto_pwhash(p, sizeof verify_and_key, password, strlen(password), metadata->salt, metadata->iterations, 1 << (metadata->memory_kbits + 10), crypto_pwhash_ALG_DEFAULT) == 0) {
			read_binary(password_verify, &p, sizeof metadata->password_verify);
			read_binary(key, &p, sizeof metadata->encrypted_key);
		} else {
			printf("The -m parameter is larger than the available memory. "); 

			if(confirm)
				printf("Please reduce -m or free some memory.");
			else
				printf("Please free some memory.");

			result = OUT_OF_MEMORY;
		}
	}

	// TODO: zero password, confirm_password, and verify_and_key
	free(password);
	free(confirm_password);
	return result;
}


size_t read_password(char **lineptr, size_t *n, FILE *stream) {
	struct termios old_term, new_term;
	int nread;

	/* Turn echoing off and fail if we can't. */
	if (tcgetattr(fileno(stream), &old_term) != 0)
		return -1;

	new_term = old_term;
	new_term.c_lflag &= ~ECHO;

	if (tcsetattr(fileno(stream), TCSAFLUSH, &new_term) != 0)
		return -1;

	/* Read the password. */
	nread = getline(lineptr, n, stream);	// TODO: this needs to zero memory

	/* Restore terminal. */
	(void) tcsetattr(fileno(stream), TCSAFLUSH, &old_term);

	return nread;
}


int check_excessive(action_type action, int memory_kbits, long iterations) {
	int result = -1;
	char *line = 0;
	size_t line_len = 0;
	const char *time_frame = "minutes";
	unsigned long long cost = (unsigned long long)iterations * (1UL << memory_kbits);
 	unsigned long long beyond_excessive = cost / EXCESSIVE_COST;

	if(!beyond_excessive)
		return 0;

	if(beyond_excessive >= (1 << 2))
		time_frame = "minutes (or hours)";
	if(beyond_excessive >= (1 << 4))
		time_frame = "hours";
	if(beyond_excessive >= (1 << 7))
		time_frame = "hours (or days)";
	if(beyond_excessive >= (1 << 9))
		time_frame = "days";
	if(beyond_excessive >= (1 << 12))
		time_frame = "days (or months)";
	if(beyond_excessive >= (1 << 14))
		time_frame = "months";
	if(beyond_excessive >= (1 << 16))
		time_frame = "months (or years)";
	if(beyond_excessive >= (1 << 18))
		time_frame = "years";

	
	printf("It may take several %s to %s this file.\n", time_frame, (action == ENCRYPT ? "encrypt" : "decrypt"));

	while(result == -1) {
		printf("Are you sure you wish to continue? (y/n)\n");

		if(getline(&line, &line_len, stdin) == -1 || line[0] == 'n' || line[0] == 'N');
			result = USER_CANCELLED;
		else if(line[0] == 'y' || line[0] == 'Y')
			result = 0;

		if(result == -1)
			printf("Please enter either 'y' or 'n'\n");
	}

	free(line);
	return result;
}


int valid_memory_kbits(int memory_kbits) {
	return memory_kbits >= MIN_MEMORY_KBITS && memory_kbits <= MAX_MEMORY_KBITS;
}


int valid_iterations(int iterations) {
	return iterations >= MIN_ITERATIONS && iterations <= MAX_ITERATIONS;
}

