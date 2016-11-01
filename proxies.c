#include "proxies.h"
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <termio.h>


int fileno(FILE *file);


FILE* (*p_fopen)(const char *path, const char *mode) = fopen;
int (*p_fclose)(FILE *file) = fclose;
int (*p_fseek)(FILE *file, long offset, int whence) = fseek;
long (*p_ftell)(FILE *file) = ftell;

int (*p_fprintf)(FILE *file, const char *format, ...) = fprintf;
size_t (*p_fread)(void *ptr, size_t size, size_t count, FILE *file) = fread;
size_t (*p_fwrite)(const void *ptr, size_t size, size_t count, FILE *file) = fwrite;
char* (*p_fgets)(char *s, int size, FILE *file) = fgets;
int (*p_fgetc)(FILE *file) = fgetc;

int (*p_fileno)(FILE *file) = fileno;
void (*p_perror)(const char *str) = perror;

int (*p_tcgetattr)(int file, struct termios *termios_p) = tcgetattr;
int (*p_tcsetattr)(int file, int actions, const struct termios *termios_p) = tcsetattr;

void* (*p_malloc)(size_t size) = malloc;
void (*p_free)(void *ptr) = free;

void (*p_randombytes_buf)(void * const buf, const size_t size) = randombytes_buf;

// maybe?
int (*p_crypto_pwhash)(unsigned char * const out, unsigned long long outlen, const char * const passwd, unsigned long long passwdlen, const unsigned char * const salt, unsigned long long opslimit, size_t memlimit, int alg) = crypto_pwhash;
int (*p_crypto_secretbox_easy)(unsigned char *ciphertext, const unsigned char *plaintext, unsigned long long plaintext_len, const unsigned char *nonce, const unsigned char *key) = crypto_secretbox_easy;
int (*p_crypto_secretbox_open_easy)(unsigned char *plaintext, const unsigned char *ciphertext, unsigned long long ciphertext_len, const unsigned char *nonce, const unsigned char *key) = crypto_secretbox_open_easy;



