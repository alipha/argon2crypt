#include "ui.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>


error_type parse_args(char **argv, int argc, action_type *action, int *memory_kbits, long *iterations) {
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


error_type usage(const char *exe) {
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


error_type derive_key(unsigned char *key, unsigned char *password_verify, const metadata_type *metadata, int confirm) {
	unsigned char verify_and_key[sizeof metadata->password_verify + sizeof metadata->encrypted_key];
	unsigned char *p = verify_and_key;
	error_type result = SUCCESS;
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


error_type check_excessive(action_type action, int memory_kbits, long iterations) {
	int result = -1;
	char *line = 0;
	size_t line_len = 0;
	const char *time_frame = "minutes";
	unsigned long long cost = (unsigned long long)iterations * (1UL << memory_kbits);
 	unsigned long long beyond_excessive = cost / EXCESSIVE_COST;

	if(!beyond_excessive)
		return SUCCESS;

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


