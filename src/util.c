#include <stdio.h>
#include <stdint.h>
#include "util.h"

char *get_symbolic_mode(mode_t mode) {
#define HAS(x) ((mode & x) == x)
	static char str[10];

	str[0] = HAS(S_IRUSR) ? 'r' : '-';                                               // User read
	str[1] = HAS(S_IWUSR) ? 'w' : '-';                                               // User write
	str[2] = HAS(S_ISUID) ? (HAS(S_IXUSR) ? 's' : 'S') : (HAS(S_IXUSR) ? 'x' : '-'); // User execute or setuid
	str[3] = HAS(S_IRGRP) ? 'r' : '-';                                               // Group read
	str[4] = HAS(S_IWGRP) ? 'w' : '-';                                               // Group write
	str[5] = HAS(S_ISGID) ? (HAS(S_IXGRP) ? 's' : 'S') : (HAS(S_IXGRP) ? 'x' : '-'); // Group execute or setgid
	str[6] = HAS(S_IROTH) ? 'r' : '-';                                               // Others read
	str[7] = HAS(S_IWOTH) ? 'w' : '-';                                               // Others write
	str[8] = HAS(S_ISVTX) ? (HAS(S_IXOTH) ? 't' : 'T') : (HAS(S_IXOTH) ? 'x' : '-'); // Others execute or sticky
	str[9] = '\0';                                                                   // null terminator

#undef HAS
	return str;
}

char *get_octal_mode(mode_t mode) {
	static char str[5];

	snprintf(str, sizeof(str), "%03o", mode & 07777);

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
	const unsigned int frac_digits = 2;    // number of fractional digits to print
	const bool trim_trailing_zeros = true; // trim trailing zeros
	size_t multiplier = metric ? 1000 : 1024;
	char *suffix = metric ? "B" : "iB";

	if (size == 0) {
		fprintf(stream, "Empty");
		return;
	}

	const char units[] = " kMGTPEZY"; // SI prefixes

	size_t frac_digits_pow = intpow10(frac_digits);

	if (multiplier > SIZE_MAX / frac_digits_pow) {
		// multiplier is too large
		fprintf(stream, "Error");
		return;
	}

	// divide by multiplier before adding fractional part to avoid overflow
	size_t place = 0;
	for (; size > SIZE_MAX / frac_digits_pow && units[place + 1];) {
		// overflow, divide by multiplier
		size /= multiplier;
		++place;
	}
	size_t new_size = size * frac_digits_pow;

	// divide by multiplier until the size is less than multiplier
	while (new_size >= multiplier * frac_digits_pow && units[place + 1]) {
		new_size /= multiplier;
		++place;
	}

	if (place == 0) {
		// print size as is
		fprintf(stream, "%zu bytes", size);
		return;
	}

	// print integer part
	fprintf(stream, "%zu", new_size / frac_digits_pow);

	new_size %= frac_digits_pow; // remove integer part
	unsigned int frac_digits_ = frac_digits;
	if (trim_trailing_zeros) {
		// divide by 10 while the last digit is 0
		while (new_size % 10 == 0 && frac_digits_ > 0 && new_size > 0) {
			new_size /= 10;
			--frac_digits_;
		}
	}

	// print fractional part
	if (new_size > 0)
		fprintf(stream, ".%0*zu", frac_digits_, new_size % frac_digits_pow);

	// print unit
	fprintf(stream, " %c%s", units[place], suffix);
}
