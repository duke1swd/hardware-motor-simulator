#include "avr/pgmspace.h"

// short names, 10 chars max
//                           "xxxxxxxxxx";
const char  ss_0[] PROGMEM = "LOG Start";
const char  ss_1[] PROGMEM = "IG IPA Ope";
const char  ss_2[] PROGMEM = "IG IPA Clo";
const char  ss_3[] PROGMEM = "IG N2O Ope";
const char  ss_4[] PROGMEM = "IG N2O Clo";
const char  ss_5[] PROGMEM = "SPARK Star";
const char  ss_6[] PROGMEM = "SPARK Stop";
const char  ss_7[] PROGMEM = "IG P Good1";
const char  ss_8[] PROGMEM = "IG P Good";
const char  ss_9[] PROGMEM = "IG Pressur";
const char ss_10[] PROGMEM = "MN N2O Chg";
const char ss_11[] PROGMEM = "MN IPA Chg";
const char ss_12[] PROGMEM = "MAIN done";
const char ss_13[] PROGMEM = "MAIN PCT";
const char ss_14[] PROGMEM = "FUEL GONE";
const char ss_15[] PROGMEM = "TIME Rollo";

const char * const op_codes_short[] PROGMEM = {
		ss_0,
		ss_1,
		ss_2,
		ss_3,
		ss_4,
		ss_5,
		ss_6,
		ss_7,
		ss_8,
		ss_9,
		ss_10,
		ss_11,
		ss_12,
		ss_13,
		ss_14,
		ss_15,
};

// long names, 20 chars max
//                           "xxxxxxxxxxxxxxxxxxxx";
const char  ls_0[] PROGMEM = "LOG Start";
const char  ls_1[] PROGMEM = "IG IPA Open";
const char  ls_2[] PROGMEM = "IG IPA Close";
const char  ls_3[] PROGMEM = "IG N2O Open";
const char  ls_4[] PROGMEM = "IG N2O Close";
const char  ls_5[] PROGMEM = "SPARK Start";
const char  ls_6[] PROGMEM = "SPARK Stop";
const char  ls_7[] PROGMEM = "IG Press First Good";
const char  ls_8[] PROGMEM = "IG Press Good";
const char  ls_9[] PROGMEM = "IG Pressure Value";
const char ls_10[] PROGMEM = "MAIN N2O Servo";
const char ls_11[] PROGMEM = "MAIN IPA Servo";
const char ls_12[] PROGMEM = "MAIN DONE";
const char ls_13[] PROGMEM = "MAIN Chamber PCT";
const char ls_14[] PROGMEM = "MAIN Fuel Gone";
const char ls_15[] PROGMEM = "TIME Rollover";

const char * const op_codes_long[] PROGMEM = {
		ls_0,
		ls_1,
		ls_2,
		ls_3,
		ls_4,
		ls_5,
		ls_6,
		ls_7,
		ls_8,
		ls_9,
		ls_10,
		ls_11,
		ls_12,
		ls_13,
		ls_14,
		ls_15,
};
