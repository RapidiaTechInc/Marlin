/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * endstops.h - manages endstops
 */

#include "../inc/MarlinConfig.h"
#include <stdint.h>

enum EndstopEnum : char {
  X_MIN,  Y_MIN,  Z_MIN,  Z_MIN_PROBE,
  X_MAX,  Y_MAX,  Z_MAX,
  X2_MIN, X2_MAX,
  Y2_MIN, Y2_MAX,
  Z2_MIN, Z2_MAX,
  Z3_MIN, Z3_MAX,
  Z4_MIN, Z4_MAX
};

#define X_ENDSTOP (X_HOME_DIR < 0 ? X_MIN : X_MAX)
#define Y_ENDSTOP (Y_HOME_DIR < 0 ? Y_MIN : Y_MAX)
#define Z_ENDSTOP (Z_HOME_DIR < 0 ? TERN(HOMING_Z_WITH_PROBE, Z_MIN, Z_MIN_PROBE) : Z_MAX)

#ifdef RAPIDIA_HEARTBEAT
// forward declaration
namespace Rapidia
{
  class Heartbeat;
}
#endif

class Endstops {

  #ifdef RAPIDIA_HEARTBEAT
    // for debugging output
    friend class Rapidia::Heartbeat;
  #endif

  public:
    #if HAS_EXTRA_ENDSTOPS
      typedef uint16_t esbits_t;
      TERN_(X_DUAL_ENDSTOPS, static float x2_endstop_adj);
      TERN_(Y_DUAL_ENDSTOPS, static float y2_endstop_adj);
      TERN_(Z_MULTI_ENDSTOPS, static float z2_endstop_adj);
      #if ENABLED(Z_MULTI_ENDSTOPS) && NUM_Z_STEPPER_DRIVERS >= 3
        static float z3_endstop_adj;
      #endif
      #if ENABLED(Z_MULTI_ENDSTOPS) && NUM_Z_STEPPER_DRIVERS >= 4
        static float z4_endstop_adj;
      #endif
    #else
      typedef uint8_t esbits_t;
    #endif

  private:
    // enabled: if false, aborts update checks.
    // enabled_globally: stored value for "enabled"; loaded from eeprom and
    // is used as the stored value during homing sequences (when enabled is set to true regardless
    // of enabled_globally.)
    static bool enabled, enabled_globally;

    // last recorded value for the endstops
    // note that if enabled is false, this will not update.
    // Otherwise, it should update rapidly (at least every temperature ISR.)
    static esbits_t live_state;

    // records if an endstop was hit at all; stays positive until cleared.
    // requires motion in the direction of that endstop in order to be set.
    // Set per-axis (as opposed to per-endstop).
    // Use X_MIN, Y_MIN, Z_MIN and Z_MIN_PROBE as BIT index
    static volatile uint8_t hit_state;

    #if ENDSTOP_NOISE_THRESHOLD
      static esbits_t validated_live_state;
      static uint8_t endstop_poll_count;    // Countdown from threshold for polling
    #endif

    // as endstop_state(), but by template argument.
    template<EndstopEnum>
    static bool _endstop_state();

    #if ENABLED(RAPIDIA_NOZZLE_PLUG_HYSTERESIS)
      // current # of z max encountered.
      static uint8_t z_max_hysteresis_count;

    public:
      // required # of detections in a row to trigger endstop.
      // (default is 1.)
      static uint8_t z_max_hysteresis_threshold;

      // previous timestamp hysteresis was updated
      // (allowed to update at most once per millisecond
      // to prevent duplicate samples)
      static uint16_t z_max_hysteresis_prev_ms;

      // hysteresis update occurs *no more rapidly than* this value.
      const static uint16_t z_max_hysteresis_min_interval_ms;

      #if ENABLED(RAPIDIA_NOZZLE_PLUG_HYSTERESIS_DEBUG_RECORDING)
        static bool z_max_hysteresis_recording;
        static millis_t z_max_hysteresis_record_begin_ms;
        static millis_t z_max_hysteresis_record_end_ms;
        static const uint8_t z_max_hysteresis_record_time_interval;
        static uint8_t* const z_max_hysteresis_record_buffer;
        static const uint16_t z_max_hysteresis_record_buffer_size;
        static uint16_t z_max_hysteresis_record_buffer_bit_index;

        static void update_z_max_hysteresis_record(bool high, uint16_t now_ms);

        static void start_z_max_hysteresis_record();

        static void z_max_hysteresis_event_update();
      #endif
    #endif

  public:
    Endstops() {};

    /**
     * Initialize the endstop pins
     */
    static void init();

    /**
     * Periodic call to poll endstops if required. Called from temperature ISR.
     * Depending on implementation, calls update() or does nothing.
     */
    FORCE_INLINE static bool abort_enabled() {
      return enabled || TERN0(HAS_BED_PROBE, z_probe_enabled);
    }

    static void poll();

    #if ENABLED(RAPIDIA_NOZZLE_PLUG_HYSTERESIS)
      // checks z_max pin and updates hysteresis count.
      // (Only safe to call from ISR context.)
      static void update_z_max_hysteresis();

      // as above, but safe to call from non-ISR context.
      static void update_z_max_hysteresis_core();
    #endif

    /**
     * Update endstops bits from the pins. Apply filtering to get a verified state.
     * If abort_enabled() and moving towards a triggered switch, abort the current move.
     * Called from ISR contexts.
     */
    static void update();

    /**
     * Get Endstop hit state.
     */
    FORCE_INLINE static uint8_t trigger_state() { return hit_state; }

    /**
     * Get current endstops state
     */
    FORCE_INLINE static esbits_t state() {
      return
        #if ENDSTOP_NOISE_THRESHOLD
          validated_live_state
        #else
          live_state
        #endif
      ;
    }

    // directly checks a particular endstop.
    static bool endstop_state(EndstopEnum);

    // Enable / disable endstop checking
    static void enable(const bool onoff=true);

    // Enable / disable endstop z-probe checking
    #if HAS_BED_PROBE
      static volatile bool z_probe_enabled;
      static void enable_z_probe(const bool onoff=true);
    #endif

    // Enable / disable endstop checking globally
    static void enable_globally(const bool onoff=true);

    static inline bool global_enabled() { return enabled_globally; }

    // Disable / Enable endstops based on ENSTOPS_ONLY_FOR_HOMING and global enable
    static void not_homing();

    #if ENABLED(VALIDATE_HOMING_ENDSTOPS)
      // If the last move failed to trigger an endstop, call kill
      static void validate_homing_move();
    #else
      FORCE_INLINE static void validate_homing_move() { hit_on_purpose(); }
    #endif

    // Clear endstops (i.e., they were hit intentionally) to suppress the report
    FORCE_INLINE static void hit_on_purpose() { hit_state = 0; }

    // Debugging of endstops
    #if ENABLED(PINS_DEBUGGING)
      static bool monitor_flag;
      static void monitor();
      static void run_monitor();
    #endif

    #if ENABLED(SPI_ENDSTOPS)
      typedef struct {
        union {
          bool any;
          struct { bool x:1, y:1, z:1; };
        };
      } tmc_spi_homing_t;
      static tmc_spi_homing_t tmc_spi_homing;
      static void clear_endstop_state();
      static bool tmc_spi_homing_check();
    #endif

    /**
     * Report endstop hits to serial. Called from loop().
     */
    static void event_handler();

    /**
     * Report endstop states in response to M119
     */
    static void report_states();

  private:

    // depending on implementation, either calls update() or waits for temperature ISR to call update().
    static void resync();
};

extern Endstops endstops;

/**
 * A class to save and change the endstop state,
 * then restore it when it goes out of scope.
 */
class TemporaryGlobalEndstopsState {
  bool saved;

  public:
    TemporaryGlobalEndstopsState(const bool enable) : saved(endstops.global_enabled()) {
      endstops.enable_globally(enable);
    }
    ~TemporaryGlobalEndstopsState() { endstops.enable_globally(saved); }
};
