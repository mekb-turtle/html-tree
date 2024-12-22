#include <stdio.h>
#include "util.h"

char *get_symbolic_mode(mode_t mode) {
	static char str[10];

	str[0] = (mode & S_IRUSR) ? 'r' : '-';                                                       // User read
	str[1] = (mode & S_IWUSR) ? 'w' : '-';                                                       // User write
	str[2] = (mode & S_ISUID) ? ((mode & S_IXUSR) ? 's' : 'S') : ((mode & S_IXUSR) ? 'x' : '-'); // User execute or setuid
	str[3] = (mode & S_IRGRP) ? 'r' : '-';                                                       // Group read
	str[4] = (mode & S_IWGRP) ? 'w' : '-';                                                       // Group write
	str[5] = (mode & S_ISGID) ? ((mode & S_IXGRP) ? 's' : 'S') : ((mode & S_IXGRP) ? 'x' : '-'); // Group execute or setgid
	str[6] = (mode & S_IROTH) ? 'r' : '-';                                                       // Others read
	str[7] = (mode & S_IWOTH) ? 'w' : '-';                                                       // Others write
	str[8] = (mode & S_ISVTX) ? ((mode & S_IXOTH) ? 't' : 'T') : ((mode & S_IXOTH) ? 'x' : '-'); // Others execute or sticky
	str[9] = '\0';                                                                               // null terminator

	return str;
}

char *get_octal_mode(mode_t mode) {
	static char str[5];

	snprintf(str, sizeof(str), "%04o", mode & 07777);

	return str;
}

size_t intpow10(size_t x) {
	size_t result = 1;
	while (x--) result *= 10;
	return result;
}

size_t intlog10(size_t x) {
	size_t result = 0;
	while (x >= 10) {
		x /= 10;
		result++;
	}
	return result;
}

void print_size(size_t size, bool metric, FILE *stream) {
	const int frac_digits_ = 2;       // number of fractional digits to print
	const char units[] = " kMGTPEZY"; // SI prefixes
	size_t multiplier = metric ? 1000 : 1024;

	int frac_digits = frac_digits_;
	size_t frac_digits_pow = intpow10(frac_digits);
	size_t new_size = size * frac_digits_pow;
	if (new_size < size) {
		// overflow, remove fractional part
		frac_digits = 0;
		frac_digits_pow = 1;
		new_size = size;
	}


	size_t place = 0;
	// divide by multiplier until the size is less than multiplier
	while (new_size >= multiplier * frac_digits_pow && units[place + 1]) {
		new_size /= multiplier;
		place++;
	}

	if (place == 0) {
		// print size as is
		fprintf(stream, "%zu bytes", size);
		return;
	}

	// print size with fractional part
	fprintf(stream, "%zu.%0*zu", new_size / frac_digits_pow, frac_digits, new_size % frac_digits_pow);

	// print unit
	fprintf(stream, " %c%sB", units[place], metric ? "" : "i");
}
