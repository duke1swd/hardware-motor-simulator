/*
 * Test routine for main valve input.
 *
 * This routine prints both main valve states on LCD.
 * For now, this status is the raw pulse width in microseconds.
 * Updates are limited to 10 per second, to avoid flickering.
 */

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "io_ref.h"
#include "state.h"
#include "menu.h"
#include "buffer.h"

extern LiquidCrystal lcd;

static unsigned long next_update_time;
extern unsigned long loop_time;
static unsigned long const update_period = 200;

void main_valve_test_state(bool first_time) {
	int dipa, dn2o;

	if (first_time) {
		lcd.clear();
		lcd.setCursor(3, 0);
		lcd.print("Main Valve Test");
		lcd.setCursor(0, 2);
		lcd.print("IPA Valve:");
		lcd.setCursor(0, 3);
		lcd.print("N2O Valve:");
		next_update_time = 0;
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

	dipa = servo_read_ipa();
	dn2o = servo_read_n2o();

	buffer_zip();
	buffer[8] = '\0';

	lcd.setCursor(12, 2);
	lcd.print(buffer);
	lcd.setCursor(12, 2);
	if (dipa == -1) 
		lcd.print("N/C");
	else if (dipa == -2)
		lcd.print("error");
	else
		lcd.print(dipa);

	lcd.setCursor(12, 3);
	lcd.print(buffer);
	lcd.setCursor(12, 3);
	if (dn2o == -1) 
		lcd.print("N/C");
	else if (dn2o == -2)
		lcd.print("error");
	else
		lcd.print(dn2o);
}
