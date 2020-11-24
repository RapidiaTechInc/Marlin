/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * MegaTronics v3.0 / v3.1 pin assignments
 */

#ifndef __AVR_ATmega2560__
  #error "Oops!  Make sure you have 'Arduino Mega' selected from the 'Tools -> Boards' menu."
#endif

#if MB(MEGATRONICS_31)
  #define BOARD_NAME "Megatronics v3.1"
#else
  #define BOARD_NAME "Megatronics v3.0"
#endif

#ifndef PIN_UNDEFINED
  #define PIN_UNDEFINED -1
#endif

//
// Servos (unused?)
//
#define SERVO0_PIN         46   // AUX3-6
#define SERVO1_PIN         47   // AUX3-5
#define SERVO2_PIN         48   // AUX3-4
#define SERVO3_PIN         49   // AUX3-3

//
// Limit Switches
//
#define X_MIN_PIN          77
#define X_MAX_PIN          79
#define Y_MIN_PIN          PIN_UNDEFINED
#define Y_MAX_PIN          33
#define Z_MIN_PIN          34
#define Z_MAX_PIN          22

//
// Z Probe (when not Z_MIN_PIN)
//
#ifndef Z_MIN_PROBE_PIN
  #define Z_MIN_PROBE_PIN Z_MIN_PIN
#endif

//
// Steppers
//
#define X_STEP_PIN         5
#define X_DIR_PIN          3
#define X_ENABLE_PIN       2

#define X2_STEP_PIN        73
#define X2_DIR_PIN         76
#define X2_ENABLE_PIN      75

#define Y_STEP_PIN         30
#define Y_DIR_PIN          32
#define Y_ENABLE_PIN       31

#define Z_STEP_PIN         25
#define Z_DIR_PIN          23
#define Z_ENABLE_PIN       24

#define E0_STEP_PIN        29
#define E0_DIR_PIN         27
#define E0_ENABLE_PIN      28

#define E1_STEP_PIN        57
#define E1_DIR_PIN         55
#define E1_ENABLE_PIN      56

//
// Temperature Sensors
//
#define TEMP_0_PIN         9   // Analog Input
#define TEMP_1_PIN         8   // Analog Input
#define TEMP_2_PIN         PIN_UNDEFINED   // Analog Input
#ifdef RAPIDIA_NO_HEATED_BED
  #define TEMP_BED_PIN       PIN_UNDEFINED
#else
  #define TEMP_BED_PIN       15   // Analog Input
#endif

//
// Heaters / Fans
//
#define HEATER_0_PIN       44
#define HEATER_1_PIN       7
#define HEATER_2_PIN       PIN_UNDEFINED
#ifdef RAPIDIA_NO_HEATED_BED
  #define HEATER_BED_PIN     PIN_UNDEFINED
#else
  #define HEATER_BED_PIN     46
#endif

//Layer Fans
#define FAN_PIN      45
#define FAN2_PIN    6

//
// Misc. Functions
//
#define SDSS               53
#define LED_PIN            PIN_UNDEFINED
#define PS_ON_PIN          PIN_UNDEFINED
#define CASE_LIGHT_PIN     45   // Try the keypad connector
