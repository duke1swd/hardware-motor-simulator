/*
 * Support functions for the digital to analog converters.
 *
 * NOTE: DAC support is outside the outputs.cpp framework.
 * The DAC is programmed inline when
 *
 * The DAC we use is a Sparkfun breakout board:
 *	https://www.sparkfun.com/products/12918
 */

#include "Arduino.h"
#include <Wire.h>
#include "pins.h"
#include "dac.h"


/*
 * This block of code is from Sparkfun's documentation:
 */

// This is the I2C Address of the MCP4725, by default (A0 pulled to GND).
// Please note that this breakout is for the MCP4725A0.
#define MCP4725_ADDR 0x60
// For devices with A0 pulled HIGH, use 0x61
//
/*
 * End of Sparkfun snippit
 */

#define	DAC_ADDR	MCP4725_ADDR

//For devices with A0 pulled HIGH, use 0x61
#define	DAC_WRITE_DAC	0x40	// write the DAC register
#define	DAC_WRITE_EE	0x60	// write the DAC and the DAC's EEPROM register
#define	DAC_PD_NORMAL	0x00	// PD bits are set for normal operation
#define	DAC_PD_OFF_LOW	0x02	// PD bits are set for power off, 1K resistor to GND
#define	DAC_PD_OFF_MED	0x04	// PD bits are set for power off, 100K resistor to GND
#define	DAC_PD_OFF_HIGH	0x06	// PD bits are set for power off, 500K resistor to GND

#define	IDLE_MAIN	410	// 4096 / 10 => 0.5 volts, idle state of sensor

/*
 * This routine sets the DAC using a 10-bit number.  Handy because input pressures
 * are 10-bits
 */
void dac_set10(int dac, int val) {
	dac_set(dac, val<<2);
}

/*
 * This routine sets the output voltage on the DAC using a 12-bit number.
 * This does not affect the power-up voltage of the DAC
 * 0 = 0V.
 * 4095 = VCC (5.0 volts)
 *
 * NOTE: parameter dac must be 0 or 1.
 */

void dac_set(int dac, int val) {
	Wire.beginTransmission(DAC_ADDR | (dac & 1));
	Wire.write(DAC_WRITE_DAC | DAC_PD_NORMAL);	// cmd to update the DAC
	Wire.write(val >> 4);				// the 8 most significant bits...
	Wire.write((val & 0x0f) << 4);			// the 4 least significant bits...
	Wire.endTransmission();
}

void dac_off(int dac) {
	Wire.beginTransmission(DAC_ADDR | (dac & 1));
	Wire.write(DAC_WRITE_DAC | DAC_PD_OFF_MED);	// cmd to update the DAC
	Wire.write(0);					// the 8 most significant bits...
	Wire.write(0);					// the 4 least significant bits...
	Wire.endTransmission();
}

/*
 * Figure out if there is an actual pressure sensor attached to the chamber sensor line.
 * Leaves the DAC_IG turned off.
 */
bool dac_ig_press_present()
{
	dac_off(DAC_IG);
	return (analogRead(PIN_IG_PRESS) < 90? false: true);
}

/*
 * Set the DACs to power up in proper state.
 */
void dac_setup() {
	Wire.begin();

	// Set the power-up state of the DACs to main at idle,
	Wire.beginTransmission(DAC_ADDR | DAC_MAIN);
	Wire.write(DAC_WRITE_EE | DAC_PD_NORMAL);	// cmd to update the DAC and EEPROM
	Wire.write(IDLE_MAIN >> 4);			// the 8 most significant bits...
	Wire.write((IDLE_MAIN & 0x0f) << 4);		// the 4 least significant bits...
	Wire.endTransmission();
	//
	// and igniter powered down at medium impedence.
	Wire.beginTransmission(DAC_ADDR | DAC_IG);
	Wire.write(DAC_WRITE_EE | DAC_PD_OFF_MED);	// cmd to update the DAC and EEPROM
	Wire.write(0);					// the 8 most significant bits...
	Wire.write(0);					// the 4 least significant bits...
	Wire.endTransmission();
}
