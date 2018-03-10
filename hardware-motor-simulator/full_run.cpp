/*
 * Do The Thing
 */

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "io_ref.h"
#include "state.h"
#include "menu.h"
#include "log.h"
#include "dac.h"
#include "pressure.h"

// amount of noise we put on simulated pressure traces.
// Should be smaller than hysteresis value (3) in inputs.cpp
#define	NOISE	2

extern LiquidCrystal lcd;
extern unsigned long loop_time;
static unsigned long next_check_time;
static unsigned long test_start_time;
static const unsigned long check_interval = 100; // milliseconds
extern void running_state(bool);

#define	IG_DELAY		25		// igniter fires 25 ms after spark + propellants

#define	PROPELLANT_LOAD		4000		// in 4 seconds of full throttle.  Units are ms.

#define	N2O_SERVO_MIN		(44+5)		// degress.  Off.
#define	N2O_SERVO_MAX		(N2O_SERVO_MIN + 80)
#define	IPA_SERVO_MIN		(44+5)		// degress.  Off.
#define	IPA_SERVO_MAX		(IPA_SERVO_MIN + 80)

//chamber behavior parameters: permit things like no/low pressure on main chamber startup,
//or failure to reach full pressure when main valves open fully
#define CHAMBER_EFF		100		// relative pressure
#define CHAMBER_MAX_PCT		100		// max pressure percentage

//igniter failure modes
#define IG_LIGHT		true
#define IG_STAY_LIT		true

bool fr_sim_ig;		// true if we are simulating the igniter pressure sensor
int chamber_p;		// simulated chamber pressure

/*
 * Common cleanup and state exit routine.
 * Called either by input_action_button or by running out of fuel
 */
static void do_exit() {
	input_action_button = false;
	dac_set10(DAC_MAIN, NO_PRESSURE);
	if (fr_sim_ig)
		dac_set10(DAC_IG, NO_PRESSURE);
	log_enabled = false;
	log_commit();
	output_led = LED_OFF;
	state_new(menu_state);
}

void full_run_state(bool first_time) {

	if (first_time) {
		log_reset();
		log_enabled = true;
		lcd.clear();
		lcd.print("Full Run");
		next_check_time = 0;
		output_led = LED_ON;
		dac_set10(DAC_MAIN, NO_PRESSURE);
	}

	if (loop_time >= next_check_time) {
		// disable this feature for now
		// first_time = true;
		next_check_time = loop_time + check_interval;
	}
	
	if (input_action_button) {
		do_exit();
		return;
	}

	if (first_time) {
		fr_sim_ig = !dac_ig_press_present();
		lcd.setCursor(0, 1);
		if (fr_sim_ig) {
			lcd.print("Simulated Ignitor");
			dac_set10(DAC_IG, NO_PRESSURE);
		} else
			lcd.print("Real Igniter     ");
	}

	if (input_ig_valve_ipa_level || input_ig_valve_n2o_level || input_spark_sense)
		state_new(running_state);
}

/*
 * Monitor the igniter.
 * Log when pressure becomes good.
 * Record when it became good so simulated main will wait awhile after igniter to work.
 *
 * This routine is called and does the right thing regardless of whether ig pressure is
 * from a real sensor or generated by the pressure sensor simulator.
 */
static bool ig_pressure_good;
static bool ig_pressure_has_been_good;
static unsigned long ig_pressure_start;
static unsigned long ig_good_time;		// Time when we think the igniter should fire.

static void monitor_ig() {
	bool b;

	b = (input_ig_press >= IG_PRESS_GOOD);
	if (b && !ig_pressure_has_been_good) {
		log(LOG_IG_PRESSURE_GOOD_1, 0);
		ig_pressure_has_been_good = true;
	} else if (b && !ig_pressure_good)
		log(LOG_IG_PRESSURE_GOOD, 0);

	if (b && !ig_pressure_good)
		ig_pressure_start = loop_time;	// set on rising edge
	ig_pressure_good = b;
}

/*
 * Simulate the ingiter.  We do this every millisecond.
 */
static unsigned long sim_ig_next_update;
static const unsigned long sim_ig_interval = 1;
static int sim_ig_output;
static int sim_ig_increment;
static int sim_ig_output_target;
static int sim_noise;

//General behavior:
//Igniter normally lights after a delay if alcohol, nitrous, and spark are present (IG_LIGHT).
//Igniter normally stays lit if alcohol and nitrous are present (IG_STAY_LIT).
//Igniter pressure is always >= chamber pressure. Igniter is lit if there is pressure.
//(AKA igniter will relight from the chamber.)
//Igniter pressure is slew limited and very slightly noisy.
static void sim_ig() {
	sim_noise++;
	if (sim_noise > NOISE)
		sim_noise = -NOISE;

	// avoid running too often
	if (loop_time < sim_ig_next_update)
		return;
	sim_ig_next_update = loop_time + sim_ig_interval;

	// If we are changing the output signal, do so gradually.
	if (sim_ig_output < sim_ig_output_target) {
		sim_ig_output += sim_ig_increment;
		if (sim_ig_output >= sim_ig_output_target) {
			sim_ig_output = sim_ig_output_target;
		}
	}
	if (sim_ig_output > sim_ig_output_target) {
		sim_ig_output -= sim_ig_increment;
		if (sim_ig_output <= sim_ig_output_target) {
			sim_ig_output = sim_ig_output_target;
		}
	}
	dac_set10(DAC_IG, sim_ig_output + sim_noise);

	// If any of the valves are off, kill the ig pressure
	if (!input_ig_valve_ipa_level || !input_ig_valve_n2o_level) {
		sim_ig_output_target = NO_PRESSURE;
	}

	// If conditions are right, and have been for awhile, ig pressure up.
	if (input_ig_valve_ipa_level && input_ig_valve_n2o_level
			&& (input_spark_sense || (chamber_p > NO_PRESSURE + 10))
			&& IG_LIGHT) {
		if (ig_good_time == 0)
			ig_good_time = loop_time + IG_DELAY;
		else if (loop_time >= ig_good_time) {
			ig_good_time = 0;
			sim_ig_output_target = IG_PRESSURE_TARGET;
		}
	}

	// If conditions are not right for ignition, don't let it start
	if (!input_ig_valve_ipa_level || !input_ig_valve_n2o_level || !input_spark_sense) {
		ig_good_time = 0;
		if (!IG_STAY_LIT) {
			sim_ig_output_target = NO_PRESSURE;
		}
	}


	if (sim_ig_output_target < chamber_p)
		sim_ig_output_target = chamber_p;
}

/*
 * Main chamber fires when propellants are present and igniter has been at pressure
 * for some minimum time.
 *
 * Each servo is converted into a percentage.  The smaller percentage is used to scale
 * the chamber pressure.
 *
 * Chamber running time is the integral of chamber pressure percentage.  System is loaded
 * with a specified amount (in seconds) of propellants.
 *
 * When propellants run out, we exit to log review state.
 *
 * Chamber pressure has efficiency (CHAMBER_EFF) and max (CHAMBER_MAX_PCT) parameters.
 * These can be used to simulate things like no ignition (CHAMBER_EFF low, maybe 5%?),
 * or other odd behavior.
 */
static unsigned long sim_main_next_update;
static const unsigned long sim_main_interval = 1;
static const unsigned int servo_slew_inv_rate = 2;	// 2 milliseconds to slew 1 degree
static unsigned long last_servo_update_time;
static int ipa_servo_pos;
static int n2o_servo_pos;
static int ipa_pct;		// percent of full flow rate that the valve is open
static int n2o_pct;		// percent of full flow rate that the valve is open
static int ipa_level;		// amount of ipa we have left
static int n2o_level;
static int ipa_fractional_consumed;	// sum of pct each ms.
static int n2o_fractional_consumed;

static void servo_slew_init() {
	last_servo_update_time = loop_time;
}

// compute the simulated servo positions.
// simulate the propellant flow rates
static void servo_slew() {
	int servo_target;
	int d;

	d = (loop_time - last_servo_update_time) / servo_slew_inv_rate;
	last_servo_update_time += d * servo_slew_inv_rate;

	servo_target = servo_read_ipa();
	if (servo_target > 0 && servo_target != ipa_servo_pos) {
		if (servo_target > ipa_servo_pos) {
			ipa_servo_pos += d;
			if (ipa_servo_pos > servo_target)
				ipa_servo_pos = servo_target;
		} else {
			ipa_servo_pos -= d;
			if (ipa_servo_pos < servo_target)
				ipa_servo_pos = servo_target;
		}
	}

	servo_target = servo_read_n2o();
	if (servo_target > 0 && servo_target != n2o_servo_pos) {
		if (servo_target > n2o_servo_pos) {
			n2o_servo_pos += d;
			if (n2o_servo_pos > servo_target)
				n2o_servo_pos = servo_target;
		} else {
			n2o_servo_pos -= d;
			if (n2o_servo_pos < servo_target)
				n2o_servo_pos = servo_target;
		}
	}
}

static int old_chamber_pct;
static unsigned long last_main_log_time;
static bool fuel_gone;

static void sim_main() {
	int chamber_pct;
	//int p;

	// avoid running too often
	if (loop_time < sim_main_next_update)
		return;
	sim_main_next_update = loop_time + sim_main_interval;

	servo_slew();

	if (ipa_servo_pos <= IPA_SERVO_MIN)
		ipa_pct = 0;
	else if (ipa_servo_pos >= IPA_SERVO_MAX)
		ipa_pct = 100;
	else {
		ipa_pct = 100 * (ipa_servo_pos - IPA_SERVO_MIN) /
			(IPA_SERVO_MAX - IPA_SERVO_MIN);
	}

	ipa_fractional_consumed += ipa_pct;
	ipa_level -= ipa_fractional_consumed / 100;
	ipa_fractional_consumed %= 100;

	if (n2o_servo_pos <= N2O_SERVO_MIN)
		n2o_pct = 0;
	else if (n2o_servo_pos >= N2O_SERVO_MAX)
		n2o_pct = 100;
	else {
		n2o_pct = 100 * (n2o_servo_pos - N2O_SERVO_MIN) /
			(IPA_SERVO_MAX - IPA_SERVO_MIN);
	}

	n2o_fractional_consumed += n2o_pct;
	n2o_level -= n2o_fractional_consumed / 100;
	n2o_fractional_consumed %= 100;

	if (n2o_level < 0 || ipa_level < 0) {
		if (!fuel_gone) {
			fuel_gone = true;
			log(LOG_FUEL_GONE, 0);
		}
		chamber_pct = 0;
	} else {
		chamber_pct = (CHAMBER_EFF * min(n2o_pct, ipa_pct)) / 100;
		chamber_pct = min(chamber_pct, CHAMBER_MAX_PCT);
	}

	if (chamber_pct == old_chamber_pct)
		return;
	old_chamber_pct = chamber_pct;

	if (loop_time - last_main_log_time > 12) {
		log(LOG_MAIN_PCT, (unsigned char)chamber_pct);
		last_main_log_time = loop_time;
	}

	chamber_p = (long)chamber_pct * (long)(MAX_MAIN_PRESSURE - SENSOR_ZERO) / 100UL + SENSOR_ZERO;
	dac_set10(DAC_MAIN, chamber_p);
}

/*
 * This state handles running the test.
 */
static unsigned long end_time;

void running_state(bool first_time) {
	extern void log_review_state(bool);

	if (input_action_button) {
		do_exit();
		return;
	}

	if (first_time) {
		output_led = LED_BLINKING;
		test_start_time = loop_time;
		ig_pressure_good = false;
		ig_pressure_has_been_good = false;
		sim_ig_output = NO_PRESSURE;		// no pressure, but sensor present.
		sim_ig_output_target = NO_PRESSURE;	// no pressure, but sensor present.
		sim_ig_next_update = 0;

		n2o_level = PROPELLANT_LOAD;
		ipa_level = PROPELLANT_LOAD;
		ipa_pct = 0;
		ipa_fractional_consumed = 0;
		n2o_pct = 0;
		n2o_fractional_consumed = 0;
		sim_main_next_update = 0;
		last_main_log_time = 0;
		servo_slew_init();
		old_chamber_pct = 0;
		chamber_p = NO_PRESSURE;
		sim_ig_increment = 150;	//igniter pressure normally changes rapidly
		fuel_gone = false;
		end_time = 0;
	}

	if (fr_sim_ig)
		sim_ig();

	monitor_ig();

	sim_main();

	if (fuel_gone && end_time == 0)
		end_time = loop_time + 2000UL;
	
	if (fuel_gone && loop_time >= end_time) {
		log(LOG_MAIN_DONE, 0);
		do_exit();
		state_new(log_review_state);
	}
}
