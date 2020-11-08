/*
 * This routine services the servo input interrupts and keeps track of
 * the servo pulse widths.
 */

#include "Arduino.h"
#include "io_ref.h"
#include "pins.h"
#include "log.h"

#define	SERVO_MIN	544UL
#define	SERVO_MAX	2400UL
#define	SERVO_ERROR	10UL
#define MIN_CHANGE	8		// minimum change that we ignore.  Min possible change is 4 uSec
#define MAX_TIME_SINCE	((unsigned char)5) // units are 16 milliseconds
#define	DEGREES_ERROR	255		// flag value

/*
 * Externally visible
 *
 * Usage:
 * 	Can just read the _degrees variable.  That gives the last
 * 	valid reading, or DEGREES_ERROR value if none.
 *
 * 	Last valid short variables contain the value (millis()>>4)&0xff
 * 	of last reading.
 */
volatile unsigned char input_ipa_servo_degrees;	// servo position in degrees
volatile unsigned char input_n2o_servo_degrees;
volatile bool ipa_servo_change;	
volatile bool n2o_servo_change;
volatile unsigned char ipa_last_valid_short;
volatile unsigned char n2o_last_valid_short;
volatile unsigned char ipa_last_last_valid_short;
volatile unsigned char n2o_last_last_valid_short;

/*
 * Internal variables.
 */
static unsigned long ipa_raise_time;
static unsigned long n2o_raise_time;
static volatile bool ipa_valid;
static volatile bool n2o_valid;
static volatile unsigned long ipa_last_valid;
static volatile unsigned long n2o_last_valid;
static bool ipa_waiting;
static bool n2o_waiting;

extern unsigned long loop_time;

// ISR for IPA servo pin change
static void ipa_isr() {
	unsigned long w;
	unsigned char c;

	// on rising edge record time and mark state
	if (digitalRead(PIN_MAIN_IPA)) {
		ipa_raise_time = micros();
		ipa_valid = true;
		return;
	}

	// on falling edge, if no rising edge, do nothing.
	if (!ipa_valid)
		return;

	ipa_valid = false;

	// Note that an out-of-range pulse width is considered a valid result
	ipa_last_valid = millis();
	c = ((ipa_last_valid >> 4) & 0xff);

	// compute the pulse width
	// unsigned math should handle wrap-around
	w = micros() - ipa_raise_time;

	// Note that each run through the ISR changes last_valid_short.
	if (c == ipa_last_valid_short)
		c--;
	ipa_last_valid_short = c;

	/*
	 * Bounds check on servo pulse width
	 */
	if (w < SERVO_MIN - SERVO_ERROR || w > SERVO_MAX + SERVO_ERROR) {
		input_ipa_servo_degrees = DEGREES_ERROR;
		return;
	}

	if (w < SERVO_MIN)
		w = SERVO_MIN;

	if (w > SERVO_MAX)
		w = SERVO_MAX;

	c = ((w - SERVO_MIN) * 180UL) / (SERVO_MAX-SERVO_MIN);
	input_ipa_servo_degrees = ((w - SERVO_MIN) * 180UL) / (SERVO_MAX-SERVO_MIN);
}

// ISR for N2O servo pin change
static void n2o_isr() {
	unsigned long w;
	unsigned char c;

	// on rising edge record time and mark state
	if (digitalRead(PIN_MAIN_N2O)) {
		n2o_raise_time = micros();
		n2o_valid = true;
		return;
	}

	// on falling edge, if no rising edge, do nothing.
	if (!n2o_valid)
		return;

	n2o_valid = false;

	// Note that an out-of-range pulse width is considered a valid result
	n2o_last_valid = millis();
	c = ((n2o_last_valid >> 4) & 0xff);

	// compute the pulse width
	// unsigned math should handle wrap-around
	w = micros() - n2o_raise_time;

	// Note that each run through the ISR changes last_valid_short.
	if (c == n2o_last_valid_short)
		c--;
	n2o_last_valid_short = c;

	/*
	 * Bounds check on servo pulse width
	 */
	if (w < SERVO_MIN - SERVO_ERROR || w > SERVO_MAX + SERVO_ERROR) {
		input_n2o_servo_degrees = DEGREES_ERROR;
		return;
	}

	if (w < SERVO_MIN)
		w = SERVO_MIN;

	if (w > SERVO_MAX)
		w = SERVO_MAX;

	c = ((w - SERVO_MIN) * 180UL) / (SERVO_MAX-SERVO_MIN);
	input_n2o_servo_degrees = ((w - SERVO_MIN) * 180UL) / (SERVO_MAX-SERVO_MIN);
}

static unsigned char old_ipa_degrees;
static unsigned char old_n2o_degrees;

void servo_setup() {
	pinMode(PIN_MAIN_IPA, INPUT);
	pinMode(PIN_MAIN_N2O, INPUT);
	ipa_valid = false;
	n2o_valid = false;
	ipa_last_valid_short = 0;
	n2o_last_valid_short = 0;
	ipa_last_last_valid_short = 0;
	n2o_last_last_valid_short = 0;
	ipa_waiting = true;
	n2o_waiting = true;
	old_ipa_degrees = DEGREES_ERROR;
	old_n2o_degrees = DEGREES_ERROR;
	attachInterrupt(digitalPinToInterrupt(PIN_MAIN_IPA), ipa_isr, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_MAIN_N2O), n2o_isr, CHANGE);
}


/*
 * Routines retrieve the servo pulse width in degrees.
 * They return -1 if no recent valid pulse width, and -2 if the pulse is too wide or too narrow
 */
unsigned char servo_read_ipa() {
	unsigned char d;

	// If we've not seen anything for awhile, say so
	if (ipa_waiting && ipa_last_valid == ipa_last_last_valid_short)
		return -1;

	ipa_waiting = false;

	if (((millis() >> 4) && 0xff) - ipa_last_valid_short >= MAX_TIME_SINCE) {
		ipa_waiting = true;
		ipa_last_last_valid_short = ipa_last_valid_short;
		return -1;
	}

	d = input_ipa_servo_degrees;

	if (abs(d - old_ipa_degrees) > 1) {
		log(LOG_MAIN_IPA_CHANGE, d);
		old_ipa_degrees = d;
	}

	if (d == DEGREES_ERROR)
		return -2;

	return d;
}

unsigned char servo_read_ipa_old() {

	if (input_ipa_servo_degrees == DEGREES_ERROR)
		return -2;

	return input_ipa_servo_degrees;
}

unsigned char servo_read_n2o() {
	unsigned char d;

	// If we've not seen anything for awhile, say so
	if (n2o_waiting && n2o_last_valid == n2o_last_last_valid_short)
		return -1;

	n2o_waiting = false;

	if (((millis() >> 4) && 0xff) - n2o_last_valid_short >= MAX_TIME_SINCE) {
		n2o_waiting = true;
		n2o_last_last_valid_short = n2o_last_valid_short;
		return -1;
	}

	d = input_n2o_servo_degrees;

	if (abs(d - old_n2o_degrees) > 1) {
		log(LOG_MAIN_N2O_CHANGE, d);
		old_n2o_degrees = d;
	}

	if (d == DEGREES_ERROR)
		return -2;

	return d;
}

unsigned char servo_read_n2o_old() {

	if (input_n2o_servo_degrees == DEGREES_ERROR)
		return -2;

	return input_n2o_servo_degrees;
}
