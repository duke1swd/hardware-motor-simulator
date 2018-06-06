extern bool input_action_button;
extern bool input_scroll_up;
extern bool input_scroll_down;
extern bool input_ig_valve_ipa;
extern bool input_ig_valve_n2o;

// device levels, rather than events
extern bool input_ig_valve_ipa_level;
extern bool input_ig_valve_n2o_level;
extern bool input_spark_sense;
extern int  input_spark_sense_A;	// analog value.  Used only in test routines.

// These are the analog input values
extern int  input_main_press;
extern int  input_ig_press;
extern int  input_ig_press_from_dac;
extern volatile int input_ipa_servo;		// pulse width in microseconds.  disable interrupt to read
extern volatile int input_n2o_servo;		// pulse width in microseconds.  disable interrupt to read

// The sole output
extern unsigned char output_led;

// LED modes
#define	LED_OFF		0
#define	LED_ON		1
#define	LED_ONE_SHOT	2	// blinks once, then is set to LED_OFF
#define	LED_BLINKING	3
#define	LED_CONTINUE	4	// internal state, never set to this.

extern unsigned char servo_read_n2o();	// returns -1, -2 or servo angle in degrees.
extern unsigned char servo_read_ipa();	// returns -1, -2 or servo angle in degrees.
extern unsigned char servo_read_n2o_old();	// returns -1, -2 or servo angle in degrees.
extern unsigned char servo_read_ipa_old();	// returns -1, -2 or servo angle in degrees.
