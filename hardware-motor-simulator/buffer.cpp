#include "buffer.h"

char buffer[BUFFER_LEN];

void buffer_zip() {
	unsigned char i;

	for (i = 0; i < BUFFER_LEN-1; i++)
		buffer[i] = ' ';
	buffer[BUFFER_LEN-1] = '\0';
}

void buffer_zip_short() {
	unsigned char i;

	for (i = 0; i < BUFFER_LEN_SHORT; i++)
		buffer[i] = ' ';
	buffer[BUFFER_LEN_SHORT] = '\0';
}

/*
 * Print an unsigned int into the buffer.
 * 4 digits, fixed width, leading zeros suppressed
 */
void buffer_print_n_i(char col, int val) {
	char i;

	for (i = 3; i >= 0; i--) {
		buffer[col + i] = '0' + (val % 10);
		val /= 10;
		if (val <= 0)
			break;
	}
}

/*
 * Print an unsigned char into the log.
 * 3 digits, fixed width, leading zeros suppressed
 */
void buffer_print_n_c(char col, unsigned char val) {
	char i;

	for (i = 2; i >= 0; i--) {
		buffer[col + i] = '0' + (val % 10);
		val /= 10;
		if (val <= 0)
			break;
	}
}
