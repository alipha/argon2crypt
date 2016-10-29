#include "ui.h"
#include "common.h"
#include "proxies.h"
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
				p_fprintf(stderr, "Not enough arguments.\n");
				return usage(argv[0]);
			}
			*memory_kbits = strtoul(argv[++i], 0, 10);
			if(valid_memory_kbits(*memory_kbits)) {
				p_fprintf(stderr, "-m must be between %d and %d. (%s and %s of memory)\n", MIN_MEMORY_KBITS, MAX_MEMORY_KBITS, MIN_MEMORY_KBITS_STR, MAX_MEMORY_KBITS_STR);
				p_fprintf(stderr, "Valid values of -m may be further constrained by\n");
				p_fprintf(stderr, "the amount of memory on your system.\n");
				return usage(argv[0]);
			}
		} else if(strcmp(arg, "-t") == 0) {
			if(i == argc - 3) {
				p_fprintf(stderr, "Not enough arguments.\n");
				return usage(argv[0]);
			}
			*iterations = strtoul(argv[++i], 0, 10);
			if(valid_iterations(*iterations)) {
				p_fprintf(stderr, "-t must be at least %d and less than %d.\n", MIN_ITERATIONS, MAX_ITERATIONS);
				return usage(argv[0]);
			}
		} else {
			p_fprintf(stderr, "Unknown argument: %s\n", arg);
			return usage(argv[0]);
		}
	}

	if(*action == UNSPECIFIED) {
		p_fprintf(stderr, "You must specify either -e or -d for encrypting or decrypting.\n");
		return usage(argv[0]);
	}

	return SUCCESS;
}


error_type usage(const char *exe) {
	p_fprintf(stderr, "\n");
	p_fprintf(stderr, "Usage: %s [-e|-d] [-m N] [-t N] source dest\n", exe);
	p_fprintf(stderr, "\n");
	p_fprintf(stderr, "Encrypts or decrypts a file with a key derived from the user-entered passphrase\n");
	p_fprintf(stderr, "using Argon2 and ChaCha20-Poly1305.\n");
	p_fprintf(stderr, "\n");
	p_fprintf(stderr, "source, dest, and -e or -d are required.\n");
	p_fprintf(stderr, "\n");
	p_fprintf(stderr, "\t-e\tEncrypts file 'source' and saves the encrypted result as 'dest'\n");
	p_fprintf(stderr, "\t-d\tDecrypts file 'source' and saves the decrypted result as 'source'\n");
	p_fprintf(stderr, "\t-m N\tSets the memory usage to 2^N KiB. (Default is 14 (16 MiB))\n");
	p_fprintf(stderr, "\t-t N\tSets the number of iterations to N. (Default is 3)\n");
	p_fprintf(stderr, "\n");
	p_fprintf(stderr, "-m and -t are ignored when decrypting. The values used to encrypt the file are used.");
	return INVALID_ARGUMENT;
}


error_type prompt_password(char *password, int confirm) {
	char confirm_password[PASSWORD_MAX_SIZE];
	error_type error = SUCCESS;

	do {
		p_fprintf(stdout, "Enter passphrase: ");

		if((error = read_password(password, PASSWORD_MAX_SIZE)))
			break;
 
		if(password[0] == '\0' || password[0] == '\n') {
			error = USER_CANCELLED;
			break;
		}

		if(!confirm)
			break;


		p_fprintf(stdout, "Confirm passphrase: ");

		if((error = read_password(confirm_password, PASSWORD_MAX_SIZE)))
			break;

		if(strcmp(password, confirm_password) == 0)
			confirm = 0;
		else
			p_fprintf(stdout, "Passphrases do not match.\n");
	} while(confirm);

	sodium_memzero(confirm_password, sizeof confirm_password);
	return error;
}


error_type read_password(char *line, size_t max_size) {
	struct termios old_term, new_term;
	error_type error;

	/* Turn echoing off and fail if we can't. */
	if (p_tcgetattr(fileno(stdin), &old_term) != 0)
		return TERMINAL_ERROR;

	new_term = old_term;
	new_term.c_lflag &= ~ECHO;

	if (p_tcsetattr(fileno(stdin), TCSAFLUSH, &new_term) != 0)
		return TERMINAL_ERROR;

	/* Read the password. */
	error = read_line(line, max_size);

	/* Restore terminal. */
	(void) p_tcsetattr(fileno(stream), TCSAFLUSH, &old_term);

	return error;
}


error_type read_line(char *line, size_t max_size) {
	size_t count;
	int ch = '\0';

	line[0] = '\0';

	if(!p_fgets(line, max_size, stdin))
		return USER_CANCELLED;

	count = strlen(line);

	if(count == max_size - 1 && line[count - 1] != '\n') {
		// discard anything longer than max_size on the line
		while(ch != EOF && ch != '\n')
			ch = p_fgetc(stdin);

		//return LINE_LENGTH_EXCEEDED;
	}

	return SUCCESS;
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

	
	p_fprintf(stdout, "It may take several %s to %s this file.\n", time_frame, (action == ENCRYPT ? "encrypt" : "decrypt"));

	while(result == -1) {
		p_fprintf(stdout, "Are you sure you wish to continue? (y/n)\n");

		if(getline(&line, &line_len, stdin) == -1 || line[0] == 'n' || line[0] == 'N');
			result = USER_CANCELLED;
		else if(line[0] == 'y' || line[0] == 'Y')
			result = 0;

		if(result == -1)
			p_fprintf(stdout, "Please enter either 'y' or 'n'\n");
	}

	p_free(line);
	return result;
}


