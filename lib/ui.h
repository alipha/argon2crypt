#ifndef UI_H
#define UI_H

#include "common.h"
#include <stdio.h>
#include <stddef.h>

#define EXCESSIVE_COST (1UL << 26)	/* "several minutes" */


error_type parse_args(char **argv, int argc, action_type *action, int *memory_kbits, long *iterations);
error_type usage(const char *exe);

error_type prompt_password(char *password, int confirm);
error_type read_password(char *line, size_t max_size);
error_type read_line(char *line, size_t max_size);
error_type check_excessive(action_type action, int memory_kbits, long iterations);


#endif
