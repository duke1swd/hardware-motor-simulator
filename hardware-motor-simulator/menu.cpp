/*
 * This code implements the top level menu.
 */

#include <Arduino.h>
#include "state.h"
#include "io_ref.h"
#include <LiquidCrystal.h>
#include "avr/pgmspace.h"
#include "buffer.h"
extern LiquidCrystal lcd;

#define	N_MENU_LINES	4

/*
 * Each menu item is a string of up to 18 characters
 * and a state function.
 *
 * This code allows users to scroll around the menu and select
 * an item.  Then the screen is blanked and the state function
 * is set to the associated state function.
 */

/*
 * Here are the menu item strings.
 */

const char  m_0[] PROGMEM = "Full Run";
const char  m_1[] PROGMEM = "Log Review";
const char  m_2[] PROGMEM = "Log to Serial";
const char  m_3[] PROGMEM = "Spark Test";
const char  m_4[] PROGMEM = "IG Valve Test";
const char  m_5[] PROGMEM = "Main Valve Test";
const char  m_6[] PROGMEM = "Ig Pressure Sensor";
const char  m_7[] PROGMEM = "Main Wave Generatr";

const char * const menu_table[] PROGMEM = {
		m_0,
		m_1,
		m_2,
		m_3,
		m_4,
		m_5,
		m_6,
		m_7,
};

/*
 * Here are the menu state actions
 */
extern void full_run_state(bool);
extern void log_review_state(bool);
extern void log_to_serial(bool);
extern void spark_test_state(bool);
extern void ig_valve_test_state(bool);
extern void main_valve_test_state(bool);
extern void ig_press_test_state(bool);
extern void main_wave_generator_state(bool);

void (*menu_state_functions[])(bool) = {
	full_run_state,
	log_review_state,
	log_to_serial,
	spark_test_state,
	ig_valve_test_state,
	main_valve_test_state,
	ig_press_test_state,
	main_wave_generator_state,
};

#define	N_MENU_ITEMS	8

static unsigned char menu_selection;	// which is the current menu item?

/*
 * Clear the screen and then draw the menu.
 */
static void i_draw_menu()
{
	unsigned char min, max;
	unsigned char i;

	lcd.clear();

	min = 0;
	if (menu_selection > 0)
		min = menu_selection - 1;
	max = min + N_MENU_LINES - 1;
	if (max >= N_MENU_ITEMS) {
		max = N_MENU_ITEMS - 1;
#if N_MENU_ITEMS >= N_MENU_LINES
		min = max - N_MENU_LINES + 1;
#endif
	}

	for (i = 0; i < N_MENU_LINES; i++) {
		if (min + i > max)
			break;
		buffer_zip_short();
		// highlight the current menu selection
		if (i + min == menu_selection)
			buffer[0] = '*';
		strcpy_P(buffer + 2, (char*)pgm_read_word(&(menu_table[i + min])));
		lcd.setCursor(0, i);
		lcd.print(buffer);
	}
}

/*
 * Called from loop to do all the work.
 */
void menu_state(bool first_time) {
	// If the action button has been hit, then switch states.
	if (input_action_button) {
		input_action_button = false;
		/*xxx*/Serial.print("Selecting menu item "); {Serial.println((int)menu_selection); delay(500);}

		state_new(menu_state_functions[menu_selection]);
		return;
	}

	// If the scroll up button has been hit, change menu state and force redraw
	if (input_scroll_up) {
		if (menu_selection > 0) {
			menu_selection--;
			first_time = true;
		}
		input_scroll_up = false;
	}

	// If the scroll down button has been hit, change menu state and force redraw
	if (input_scroll_down) {
		if (menu_selection < N_MENU_ITEMS-1) {
			menu_selection++;
			first_time = true;
		}
		input_scroll_down = false;
	}

	if (first_time)
		i_draw_menu();
}

void menu_init() {
	menu_selection = 0;
	state_new(menu_state);
}
