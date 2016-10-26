#ifndef UI_H
#define UI_H

#include "crypto.h"
#include "common.h"
#include <stdio.h>
#include <stdint.h>

#define EXCESSIVE_COST (1UL << 26)	/* "several minutes" */


error_type parse_args(char **argv, int argc, action_type *action, int *memory_kbits, long *iterations);
error_type usage(const char *exe);

error_type derive_key(unsigned char *key, unsigned char *password_verify, const metadata_type *metadata, int confirm);
size_t read_password(char **lineptr, size_t *n, FILE *stream);
error_type check_excessive(action_type action, int memory_kbits, long iterations);


#endif
