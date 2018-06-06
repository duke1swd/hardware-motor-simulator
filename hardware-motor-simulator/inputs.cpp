/*
 * Handle all the input processing.
 * All of the event-type inputs are edge triggered.
 * 	These include the action button, the scroll switch,
 * 	and the two solenoid valves.
 *
 * 	When they are actuated the boolean event variable is set true.
 * 	The event consumer must set the event variable false when
 * 	done consuming the event, otherwise subsequent events are ignored.
 *
 * 	The action button and the scroll switch are debounced, so the
 * 	event is delayed by 10 ms, and on-times of less than 10 ms are
 * 	ignored.
 *
 *	For now the solenoids are not debounced.  Event logging code
 *	Will have to deal with this fact.  Or maybe the relays don't bounce.
 *
 * The two analog inputs are raw 10-bit values.
 *	These values are not filtered, we do add some simple hysteresis.
 *	If the value read off the input A/D has not changed by +/- 3, we
 *	don't change the reported value.  This should provide some simple
 *	noise filtering.
 *
 * The spark sensor input is converted to a digital value, but
 * 	the analog value is available too.  The digital value is neither
 * 	debounced nor edge triggered.  It is set both true and false here.
 *
 * NOTE:
 * 	Input status changes are logged here.  Whether they really go in the
 * 	log is a function of the global boolean log_enabled
 *
 * NOTE:
 * 	Various thresholds are hard coded into the input routines.
 * 	This simplifies the code by not cluttering things with a bunch
 * 	of #defines that are used exactly once.  Read the code if you
 * 	want to see the thresholds.
 * 	This applies to scroll switch and spark sense inputs.
 */

#include "Arduino.h"
#include "io_ref.h"
#include "pins.h"
#include "log.h"

extern unsigned long loop_time;

// these are the events.  They are set true here, and set
// false when consumed by the consumer
bool input_action_button;
bool input_scroll_up;
bool input_scroll_down;
bool input_ig_valve_ipa;
bool input_ig_valve_n2o;

// These are the input levels.  Not events.  Read only outside this module.
bool input_spark_sense;
bool input_ig_valve_ipa_level;
bool input_ig_valve_n2o_level;
int  input_spark_sense_A;	// analog value.  Used only in test routines.

// These are the analog input values.  They are set here only.
int  input_main_press;
int  input_ig_press;
int  input_ig_press_from_dac;

// Action button variables
static bool action_button_debounce;
static bool action_button_old_state;
static unsigned long action_button_debounce_time;

// Scroll switch variables
static bool scroll_debounce;
static unsigned char scroll_old_state;
static unsigned long scroll_debounce_time;

// Ig Valve variables
static bool ig_valve_ipa_old_state;
static bool ig_valve_n2o_old_state;

// Pressor sensor variables
// (none needed)

const static unsigned long debounce_time = 10;	// milliseconds
const static int hysteresis = 10;		// counts

static void i_action_button() {
	boolean v;

	// v is true if the button is pressed
	v = (digitalRead(PIN_ACTION) == 0);
	
	// Rising edge?
	if (v && !action_button_old_state) {
		action_button_debounce = true;
		action_button_debounce_time = loop_time + debounce_time;
	// True and debounce period over?
	} else if (v && action_button_debounce &&
			loop_time > action_button_debounce_time) {
		input_action_button = true;
		action_button_debounce = false;
	}

	// If button not pressed cancel any pending debounced rising edge
	if (!v)
		action_button_debounce = false;

	action_button_old_state = v;
}

static void i_scroll_switch() {
	int t;
	unsigned char v;

	// v is true if switch pressed either way
	t = analogRead(PIN_SCROLL);
	if (t < 10)
		v = 1;
	else if (t > 1000)
		v = 2;
	else
		v = 0;

	// Rising edge?
	if (v && v != scroll_old_state) {
		scroll_debounce = true;
		scroll_debounce_time = loop_time + debounce_time;
	// Debounce period over?
	} else if (v && scroll_debounce &&
			loop_time > scroll_debounce_time) {
		if (v == 1)
			input_scroll_up = true;
		else
			input_scroll_down = true;
		scroll_debounce = false;
	}

	// If the switch is center-off cancel any pending debounce1G
	if (!v)
		scroll_debounce = false;
	scroll_old_state = v;
}

static void i_ig_valve_ipa() {

	// true if the solenoid is actuated
	input_ig_valve_ipa_level = (digitalRead(PIN_IG_IPA) == 1);

	if (input_ig_valve_ipa_level  && !ig_valve_ipa_old_state) {
		input_ig_valve_ipa = true;
		log(LOG_IG_IPA_OPEN, 0);
	}

	if (!input_ig_valve_ipa_level  && ig_valve_ipa_old_state)
		log(LOG_IG_IPA_CLOSE, 0);

	ig_valve_ipa_old_state = input_ig_valve_ipa_level;
}

static void i_ig_valve_n2o() {
	// true if the solenoid is actuated
	input_ig_valve_n2o_level = (digitalRead(PIN_IG_N2O) == 1);

	if (input_ig_valve_n2o_level && !ig_valve_n2o_old_state) {
		input_ig_valve_n2o = true;
		log(LOG_IG_N2O_OPEN, 0);
	}

	if (!input_ig_valve_n2o_level  && ig_valve_n2o_old_state)
		log(LOG_IG_N2O_CLOSE, 0);

	ig_valve_n2o_old_state = input_ig_valve_n2o_level;
}

// the main pressure sensor isn't really in input.
// this is always connected to the output of the DAC
static void i_main_press() {
	int v, t;

	v = analogRead(PIN_MAIN_PRESS);
	t = v - input_main_press;
	if (t >= hysteresis || t <= -hysteresis) {
		input_main_press = v;
		// Don't log this.  Not worth it
		// log(LOG_MAIN_PRESSURE_CHANGE, 0xff & (v >> 2));
	}
}

static void i_ig_press() {
	int v, t;

	v = analogRead(PIN_IG_PRESS_REAL);
	t = v - input_ig_press;
	if (t >= hysteresis || t <= -hysteresis) {
		input_ig_press = v;
		log(LOG_IG_PRESSURE_CHANGE, 0xff & (v >> 2));
	}

	v = analogRead(PIN_IG_PRESS_SIM);
	t = v - input_ig_press_from_dac;
	if (t >= hysteresis || t <= -hysteresis) {
		input_ig_press = v;
	}
}

static void i_spark_sense() {
	bool b;

	input_spark_sense_A = analogRead(PIN_SPARK);
	b = (input_spark_sense_A > 50 && input_spark_sense_A < 1024-50);

	if (b && !input_spark_sense)
		log(LOG_SPARK_FIRST, 0);
	else if (!b && input_spark_sense)
		log(LOG_SPARK_LAST, 0);

	input_spark_sense = b;
}

void input_setup() {
	pinMode(PIN_ACTION, INPUT_PULLUP);
	action_button_old_state = false;
	input_action_button = false;

	pinMode(PIN_SCROLL, INPUT);
	scroll_old_state = 0;
	input_scroll_up = false;
	input_scroll_down = false;

	pinMode(PIN_IG_IPA, INPUT);
	ig_valve_ipa_old_state = false;
	input_ig_valve_ipa = false;

	pinMode(PIN_IG_N2O, INPUT);
	ig_valve_n2o_old_state = false;
	input_ig_valve_n2o = false;

	pinMode(PIN_MAIN_PRESS, INPUT);
	input_main_press = 0;

	pinMode(PIN_IG_PRESS_REAL, INPUT);
	input_ig_press = 0;

	pinMode(PIN_IG_PRESS_SIM, INPUT);
	input_ig_press_from_dac = 0;

	pinMode(PIN_SPARK, INPUT);
	input_spark_sense = 0;
	input_spark_sense_A = 0;
}

void inputs() {
	i_action_button();
	i_scroll_switch();
	i_ig_valve_ipa();
	i_ig_valve_n2o();
	i_main_press();
	i_ig_press();
	i_spark_sense();
}
