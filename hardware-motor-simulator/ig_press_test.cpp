/*
 * Test routine for ig pressure input.
 *
 * This routine prints both the raw and scaled ig pressure sensor value
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
static unsigned long const update_period = 100;
static int previous_ig_press;

void ig_press_test_state(bool first_time) {
	long c;

	if (first_time) {
		lcd.clear();
		lcd.setCursor(3, 0);
		lcd.print("Ig Pressure Test");
		lcd.setCursor(0, 2);
		lcd.print("   Raw Value:");
		lcd.setCursor(0, 3);
		lcd.print("Scaled Value:");
		next_update_time = 0;
		previous_ig_press = -2;
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

	buffer_zip();
	buffer[6] = '\0';
	lcd.setCursor(14, 2);
	lcd.print(buffer);
	lcd.setCursor(14, 3);
	lcd.print(buffer);

	lcd.setCursor(14, 2);
	if (dac_ig_press_present()) {
		lcd.print(input_ig_press);
		if (input_ig_press != previous_ig_press) {
			previous_ig_press = input_ig_press;

			c = input_ig_press - SENSOR_ZERO;
			//if (c < 0)
				//c = 0;
			c = (c * (long)PSI_RANGE * PSI_RANGE) / ((long)(SENSOR_MAX-SENSOR_ZERO));

			lcd.setCursor(14, 3);
			lcd.print(c);
		}
	} else {
		if (previous_ig_press != -1) {
			lcd.print("N/C");
			previous_ig_press = -1;
		}
	}
}
