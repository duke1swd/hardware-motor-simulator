/*
 *  This module manages the log
 *
 *  The log resides in eeprom and in main memory.
 *  On initialization valid log entries in eeprom
 *  are read into main memory.
 *
 *  The log is written to eeprom using log_commit()
 *
 *  Each log entry is 4 bytes.  See log.h for details.
 *  In addition to reading and writing the log,
 *  This code also converts log entries into strings
 *  for display on LCD and printing on serial port.
 */

#include "log.h"
#include "log_op_names.h"
#include "ee.h"
#include "EEPROM.h"
#include "buffer.h"
#include "Arduino.h"

bool log_enabled;

extern unsigned long loop_time;
extern unsigned long loop_counter;
static unsigned long log_base_time;
static int n_log_entries;
struct log_entry_s log_in_memory[LOG_SIZE];	// the in-memory copy of the log
static unsigned int log_sequence_number;
static unsigned char log_sequence_increment;

/*
 * Read the log from EEPROM, if EEPROM is available
 */
void log_init() {
	int cb2;
	int i;
	int p;

	log_enabled = false;

	log_sequence_number = EEPROM.read(LOG_SEQ0);
	log_sequence_number |= ((unsigned int)(EEPROM.read(LOG_SEQ1)) << 8);
	log_sequence_increment = 1;

	n_log_entries = EEPROM.read(LOG_CHECK_BYTE_1);
	/*xxx*/Serial.print("Read "); Serial.print(n_log_entries); Serial.print(" from EEPROM");
	cb2 = EEPROM.read(LOG_CHECK_BYTE_2);

	// check if log in eeprom is valid.
	if (n_log_entries + cb2 != LOG_SIZE ||
	    n_log_entries <= 0 ||
	    n_log_entries >= LOG_SIZE) {
		n_log_entries = 0;
		return;
	}

	p = LOG_BASE;
	for (i = 0; i < n_log_entries; i++) {
		log_in_memory[i].log_op = EEPROM.read(p++);
		log_in_memory[i].log_param = EEPROM.read(p++);
		log_in_memory[i].timestamp = EEPROM.read(p++);
		log_in_memory[i].timestamp |= (EEPROM.read(p++) << 8);
	}
}

void log_commit() {
	int cb2;
	int i;
	int p;

	// If the in-memory log is empty, don't disturb the EEPROM version.
	if (n_log_entries == 0)
		return;

	// make the log invalid
	cb2 = EEPROM.read(LOG_CHECK_BYTE_2);
	if (cb2 + n_log_entries == LOG_SIZE)
		EEPROM.write(LOG_CHECK_BYTE_2, LOG_SIZE + 1);

	EEPROM.write(LOG_CHECK_BYTE_1, n_log_entries);
	p = LOG_BASE;
	for (i = 0; i < n_log_entries; i++) {
		EEPROM.write(p++, log_in_memory[i].log_op);
		EEPROM.write(p++, log_in_memory[i].log_param);
		EEPROM.write(p++, (log_in_memory[i].timestamp) & 0xff);
		EEPROM.write(p++, (log_in_memory[i].timestamp >> 8) & 0xff);
	}
	EEPROM.write(LOG_CHECK_BYTE_2, LOG_SIZE - n_log_entries);

	EEPROM.write(LOG_SEQ0, 0xff & log_sequence_number);
	EEPROM.write(LOG_SEQ1, 0xff & (log_sequence_number >> 8));
	log_sequence_increment = 1;
}

/*
 * Erase the in-memory log
 */
void log_reset() {
	n_log_entries = 0;
	log_sequence_number += log_sequence_increment;
	log_sequence_increment = 0;
}

static void i_log(unsigned char op, unsigned char param)  {
	log_in_memory[n_log_entries].log_op = op;
	log_in_memory[n_log_entries].log_param = param;
	log_in_memory[n_log_entries].timestamp = loop_time - log_base_time;
	n_log_entries++;
}

void log(unsigned char op, unsigned char param) {
	int max;

	if (!log_enabled)
		return;

	switch(LOG_LEVEL(op)) {
		case LOG_CRITICAL:
			max = LOG_SIZE;
			break;
		case LOG_NORMAL:
			max = LOG_MAX_NORMAL;
			break;
		default:
			max = LOG_MAX_DETAIL;
			if (n_log_entries == 0)
				return;		// don't clutter before things start
			break;
	}

	if (n_log_entries >= max)
		return;
	
	if (n_log_entries == 0) {
		log_base_time = loop_time;
		loop_counter = 0;
		i_log(LOG_START, 0);
	}

	// process time stamp rollover
	while (loop_time - log_base_time > LOG_ROLLOVER_TIME) {
		if (n_log_entries < LOG_SIZE - 1)
			i_log(LOG_TIME_ROLLOVER, 0);
		log_base_time += LOG_ROLLOVER_TIME;
	}
	i_log(op, param);
}

/*
 * These routines get information out of the log in the form of printable strings.
 * Notes:
 * 	All of these routines return a pointer to the same static output buffer.
 * 	Strings are null terminated, and do not have a newline.
 * 	All strings are 20 characters wide, or less, except for routines with "long"
 * 	in their name.
 */

/*
 * Very Dangerous String Copy routine
 */
static void i_strcpy(char *p, const char *q) {
	while ( (*p++ = *q++) ) ;
}

/*
 * Return the log sequence number as a printable string
 */
char *log_tos_seqn() {
	buffer_zip();

	i_strcpy(buffer, "Log #:");
	buffer[6] = ' ';
	buffer_print_n_i(7, log_sequence_number);

	//buffer[12] = '\0';
	return buffer;
}

/*
 * Put the timestamp into the long string
 * Puts "END" if off the end.
 *
 * Returns true of nothing to print, else false.
 */
static bool i_log_tos(unsigned char entry) {
	unsigned char bias;
	unsigned char i;

	buffer_zip();

	if (entry == n_log_entries) {
		buffer[0] = 'E';
		buffer[1] = 'N';
		buffer[2] = 'D';
		return true;
	}

	if (entry > n_log_entries) {
		buffer[0] = '\0';
		return true;
	}

	bias = 0;
	for (i = 0; i < entry; i++)
		if (log_in_memory[i].log_op == LOG_TIME_ROLLOVER)
			bias++;

	if (bias > 9)
		buffer[0] = '*';
	else if (bias)
		buffer[0] = '0' + bias;
	buffer_print_n_i(1, log_in_memory[entry].timestamp + (bias? 10000: 0));

	return false;
}

/*
 * NOTE: DOES NOT NULL TERMINATE
 */
static void i_opcode_print(const char * const table[], unsigned char op) {
	char *p;

	// Necessary casts and dereferencing, just copy.
	p = buffer + 6;
	strcpy_P(p, (char*)pgm_read_word(&(table[op])));
	while (*p)
		p++;
	*p = ' ';
}

/*
 * Return a log entry as a printable string, exactly 20 characters long
 * Format:
 *  nnnnn_ssssssssss_ppp
 * Where nnnnn is the time stamp
 *       ssssssssss is the opcode as a sting
 *       ppp is the parameter
 */
char *log_tos_short(unsigned char entry) {
	unsigned char p;
	bool f;

	f = i_log_tos(entry);
	buffer[20] = '\0';

	if (f)
		return buffer;

	i_opcode_print(op_codes_short, (log_in_memory[entry].log_op) & ~LOG_LEVEL_MASK);

	p = log_in_memory[entry].log_param;
/*xxx*/ //Serial.print("op = "); Serial.print((int)(log_in_memory[entry].log_op)); Serial.print(" p = ");Serial.print((int)(p)); Serial.print("\n");
	if (p)
		buffer_print_n_c(17, p);

	return buffer;
}

/*
 * Return a log entry as a printable string, exactly 30 characters long
 * Format:
 *  nnnnn_ssssssssss_ppp
 * Where nnnnn is the time stamp
 *       ssssssssssssssssssss is the opcode as a sting
 *       ppp is the parameter
 */
char *log_tos_long(unsigned char entry) {
	unsigned char p;
	bool f;

	f = i_log_tos(entry);
	buffer[30] = '\0';

	if (f)
		return buffer;

	i_opcode_print(op_codes_long, (log_in_memory[entry].log_op) & ~LOG_LEVEL_MASK);

	p = log_in_memory[entry].log_param;
	if (p)
		buffer_print_n_c(27, p);

	return buffer;
}
