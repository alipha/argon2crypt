#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>


void read_binary(unsigned char *dest, const unsigned char **src, size_t amount);
void write_binary(unsigned char **dest, const unsigned char *src, size_t amount);
long read_hex(const unsigned char **str, size_t digits);
void write_hex(unsigned char **dest, long value, size_t digits);

void xor_bytes(unsigned char *dest, const unsigned char *src, size_t amount);


#endif
