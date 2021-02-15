#pragma once

// target configurations
//#define EMULATOR

#define RAPIDIA

#define RAPIDIA_PROTOCOL 2

#ifdef RAPIDIA_DRY
  #define RAPIDIA_NO_HOTENDS
  #define RAPIDIA_NO_EXTRUDE
  #define RAPIDIA_NO_HEATED_BED
#endif

#ifndef RAPIDIA_PLASTIC
    #define RAPIDIA_METAL
#endif

// rapidia features

// planner blocks are marked with their gcode source line
// (only for the last block in any given move)
#define RAPIDIA_BLOCK_SOURCE

// allow R730/R731 for enabling gcode line complete auto-reporting.
#define RAPIDIA_LINE_AUTO_REPORTING

// pause feature R751/752 is enabled (requires RAPIDIA_BLOCK_SOURCE)
#define RAPIDIA_PAUSE

// M736/M737 alias for M106/M107
#define RAPIDIA_LAMP_ALIAS

// Z_MAX_PIN (nozzle plug pin) requires N positive reads in a row to trigger.
// (N is user-specified.)
#define RAPIDIA_NOZZLE_PLUG_HYSTERESIS

// record nozzle state for debugging purposes.
// #define RAPIDIA_NOZZLE_PLUG_HYSTERESIS_DEBUG_RECORDING

// this is intended for debugging only.
// M codes which match the value of an R code will be interpreted as that R code.
// This allows use with hosts that don't support R codes.
// Caveat: if an M code already has semantics, it will not be interpreted as the associated R code!
//#define RAPIDIA_M_CODE_COMPATABILITY

//#define RAPIDIA_PAUSE_DEBUG

// receiving byte \x04 ("End Of Transmission") over serial (at any point) causes
// M112 (emergency stop) to be triggered.
#define RAPIDIA_EOT_EMERGENCY_STOP

// changes M112 behaviour to kill immediately within emergency parser interrupt,
// rather than waiting for the next idle() loop.
#define RAPIDIA_EMERGENCY_STOP_INTERRUPT

// allows M112 emergency stop to be recovered from
// using R750
#define RAPIDIA_KILL_RECOVERY

// allows emulator to hook into certain functionality.
// cost is minimal, so it should be left in on all configurations for consistency.
#define RAPIDIA_EMULATOR_HOOKS

// enables some development features,
// adds R8XX codes, which are for testing / development
#define RAPIDIA_DEV

// keep track of when homing is occurring
// and report it during heartbeat.
#define RAPIDIA_HOMING_SEMAPHORE

// R745, clears homing state
#define RAPIDIA_HOMING_RESET

// allow computing checksums for heartbeat, etc.
#define RAPIDIA_CHECKSUMS

// keep track of and store certain printer statistics
// (namely cumulative total extrusion amount)
//#define RAPIDIA_MILEAGE

// must be enough room for the other eeprom settings to fit before this.
#define RAPIDIA_MILEAGE_EEPROM_START 0x400

// saving every 100 seconds, we can (only) support 150 days of EEPROM writing.
// (EEPROM safe for ~100,000 writes.)
// (measured in seconds)
#define RAPIDIA_MILEAGE_SAVE_INTERVAL 100

// this multiplies the lifetime of the mileage eeprom data,
// at the cost of using this many times redundant eeprom.
#define RAPIDIA_MILEAGE_SAVE_MULTIPLICITY 100

// performs some stack monitoring
#if ENABLED(RAPIDIA_DEV)
  #define RAPIDIA_STACK_UTIL
  #define RAPIDIA_STACK_USAGE
#endif