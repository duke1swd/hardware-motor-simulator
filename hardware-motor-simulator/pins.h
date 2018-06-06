/*
 * This file records how all the pins are wired.
 * NOTE: Must include Arduino.h before this file.
 */

#ifndef _PINS
#define _PINS

#define	PIN_RX			0	// not used.
#define	PIN_TX			1	// not used.
#define	PIN_MAIN_N2O		2	// PWM/interrupt input
#define	PIN_MAIN_IPA		3	// PWM/interrupt input
#define	PIN_IG_IPA		4	// digital input
#define	PIN_IG_N2O		5	// digital input
#define	PIN_BACKLIGHT		6	// PWM output
#define	PIN_LCD_D4		7	// digital output
#define	PIN_LCD_D5		8	// digital output
#define	PIN_LCD_D6		9	// digital output
#define	PIN_LCD_D7		10	// digital output
#define	PIN_LCD_RS		11	// digital output
#define	PIN_LCD_EN		12	// digital output
#define	PIN_LED			13	// digital output

#define	PIN_MAIN_PRESS		A0	// analog input
#define	PIN_IG_PRESS_SIM	A1	// analog input
#define	PIN_SPARK		A2	// analog input
#define	PIN_ACTION		A3	// digital input
#define	PIN_I2C_SDA		A4	// digital i/o for I2C bus
#define	PIN_I2C_SCL		A5	// digital i/o for I2C bus
#define	PIN_SCROLL		A6	// analog input
#define	PIN_IG_PRESS_REAL	A7	// analog input

#endif
