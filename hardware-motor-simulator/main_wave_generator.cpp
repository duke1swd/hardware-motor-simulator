/*
 * Generate an interesting waveform on the mainpressor sensor line
 */

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "io_ref.h"
#include "state.h"
#include "menu.h"
#include "buffer.h"
#include "dac.h"
#include "pressure.h"

extern LiquidCrystal lcd;

static unsigned long next_update_time;
extern unsigned long loop_time;
static unsigned long const update_period = 1;

static const int period = 75;
static const int magnitude = 1000;
static const int min = 12;
static int iteration;

void main_wave_generator_state(bool first_time) {
	long l;

	if (first_time) {
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Main Wave Generator");
		lcd.setCursor(0, 2);
		lcd.print("   Period ");
		lcd.print(period);
		next_update_time = loop_time;
		iteration = 0;
	}
	
	// Exit the test when the action button is pressed.
	if (input_action_button) {
		input_action_button = false;
		state_new(menu_state);
		return;
	}

	// If not read to update, done.
	if (loop_time < next_update_time)
		return;

	// schedule next update.
	next_update_time = loop_time + update_period;

	l = magnitude;
	l *= iteration;
	l /= period;
	l += min;
	dac_set10(DAC_MAIN, (int)l);

	iteration += 1;
	if (iteration >= period)
		iteration = 0;
}
