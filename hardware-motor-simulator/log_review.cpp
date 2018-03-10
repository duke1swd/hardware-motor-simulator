/*
 * Display the log on the LCD screen
 */

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "io_ref.h"
#include "state.h"
#include "menu.h"
#include "log.h"
#include "buffer.h"

extern LiquidCrystal lcd;

static int lr_min;

static void i_draw() {
	unsigned char i;
	char *p;

	// draw the screen
	for (i = 0; i < 4; i++) {
		lcd.setCursor(0, i);
		if (i + lr_min < 0)
			p = log_tos_seqn();
		else
			p = log_tos_short(i + lr_min);
		if (*p) {
			p[21] = '\0';
			lcd.print(p);
		} else {
			buffer_zip_short();
			lcd.print(buffer);
		}
	}
}

void log_review_state(bool first_time) {
	bool draw_screen;

	draw_screen = false;

	if (first_time) {
		lr_min = -1;
		draw_screen = true;
	}

	if (input_action_button)  {
		input_action_button = false;
		output_led = LED_OFF;
		state_new(menu_state);
	}

	if (input_scroll_up) {
		input_scroll_up = false;
		if (lr_min > -1) {
			lr_min--;
			draw_screen = true;
		}
	}

	if (input_scroll_down) {
		input_scroll_down = false;
		if (*log_tos_short(lr_min+4)) {
			lr_min++;
			draw_screen = true;
		}
	}

	if (draw_screen) {
		output_led = LED_ONE_SHOT;
		i_draw();
	}
}

void log_to_serial(bool first_time) {
	char *p;
	
	// Exit back to the menu when the action button is pressed.
	if (input_action_button) {
		input_action_button = false;
		state_new(menu_state);
		return;
	}

	if (first_time) {
		lr_min = -1;
		lcd.clear();
		lcd.print("  Log to Serial");
	}

	if (lr_min < 0)
		p = log_tos_seqn();
	else
		p = log_tos_long((unsigned char)lr_min);
	lr_min++;
	/*xxx*/Serial.print(" -- ");Serial.print(lr_min);Serial.print("\n");

	if (*p) {
		Serial.print(p);
		Serial.print('\n');
	} else
		state_new(menu_state);
}

