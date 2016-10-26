#ifndef PROXIES_H
#define PROXIES_H

#include <stdio.h>
#include <stdint.h>
#include <termio.h>


extern FILE* (*p_fopen)(const char *path, const char *mode);
extern int (*p_fclose)(FILE *file);
extern int (*p_fseek)(FILE *file, long offset, int whence);
extern long (*p_ftell)(FILE *file);

extern int (*p_fprintf)(FILE *file, const char *format, ...);
extern size_t (*p_fread)(void *ptr, size_t size, size_t count, FILE *file);
extern size_t (*p_fwrite)(const void *ptr, size_t size, size_t count, FILE *file);
extern char* (*p_fgets)(char *s, int size, FILE *file);
extern int (*p_fgetc)(FILE *file);

extern int (*p_fileno)(FILE *file);
extern int (*p_perror)(const char *str);

extern int (*p_tcgetattr)(int file, struct termios *termios_p);
extern int (*p_tcsetattr)(int file, int actions, const struct termios *termios_p);

extern void* (*p_malloc)(size_t size);
extern void (*p_free)(void *ptr);

extern void (*p_randombytes_buf)(void * const buf, const size_t size);

// maybe?
extern int (*p_crypto_pwhash)(unsigned char * const out, unsigned long long outlen, const char * const passwd, unsigned long long passwdlen, const unsigned char * const salt, unsigned long long opslimit, size_t memlimit, int alg);
extern int (*p_crypto_secretbox_easy)(unsigned char *ciphertext, const unsigned char *plaintext, unsigned long long plaintext_len, const unsigned char *nonce, const unsigned char *key);
extern int (*p_crypto_secretbox_open_easy)(unsigned char *plaintext, const unsigned char *ciphertext, unsigned long long ciphertext_len, const unsigned char *nonce, const unsigned char *key);


#endif
