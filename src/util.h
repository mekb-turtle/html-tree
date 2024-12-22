#ifndef UTIL_H
#define UTIL_H
#include <sys/stat.h>
#include <stdbool.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

struct options {
	const char *title;
	bool metric, dotfiles, reverse, alt_mode;
};

char *get_symbolic_mode(mode_t mode);
char *get_octal_mode(mode_t mode);

size_t intpow10(size_t x);
size_t intlog10(size_t x);

void print_size(size_t size, bool metric, FILE *stream);
#endif // UTIL_H
