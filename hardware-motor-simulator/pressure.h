/*
 * The Arduino has a 10-bit DAC.
 * 0 volts = 0
 * 5 volts = 1024
 *
 * The sensor reads 0 PSI at 0.5 volts and
 * 500 PSI at 4.5 volts.
 */

#define	SENSOR_ZERO		102		// 0.5 V = 1024 / 10
#define	SENSOR_MAX		922		// 5.0 V = .9 * 1024
#define	PSI_RANGE		500

#define	NO_PRESSURE		SENSOR_ZERO	// approx 0 PSI

// 1024/10 + 75 * (8/10 * 1024) / 500
#define	IG_PRESS_GOOD		225		// approx 75 PSI

// 1024/10 + 150 * (8/10 * 1024) / 500
#define	IG_PRESSURE_TARGET	348		// approx 150 PSI

// 1024 / 10 + 200 * (8/10 * 1024) / 500
#define	MAX_MAIN_PRESSURE	430		// approx 200 PSI

// 10 * (8/10 * 1024) / 500
#define	PRESSURE_FAULT_DELTA	17		// about 10 PSI 
