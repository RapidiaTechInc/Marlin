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

#include "../../inc/MarlinConfig.h"

#include "../gcode.h"

#include "../../module/stepper.h"
#include "../../module/endstops.h"

#if HAS_MULTI_HOTEND
  #include "../../module/tool_change.h"
#endif

#if HAS_LEVELING
  #include "../../feature/bedlevel/bedlevel.h"
#endif

#include "../../module/probe.h"

#define DEBUG_OUT ENABLED(DEBUG_LEVELING_FEATURE)
#include "../../core/debug_out.h"

void do_t1_homing_move(const AxisEnum axis, const float distance, const feedRate_t fr_mm_s=0.0) {
  DEBUG_SECTION(log_move, "do_t1_homing_move", DEBUGGING(LEVELING));

  const feedRate_t real_fr_mm_s = fr_mm_s ?: homing_feedrate(axis);

  // Only do some things when moving towards an endstop
  const int8_t axis_home_dir = TERN0(DUAL_X_CARRIAGE, axis == X_AXIS)
                ? x_home_dir(active_extruder) : home_dir(axis);
  const bool is_moving_towards_endstop = (axis_home_dir > 0) == (distance > 0);

  if (is_moving_towards_endstop) {
    #if HOMING_Z_WITH_PROBE && QUIET_PROBING
      if (axis == Z_AXIS) probe.set_probing_paused(true);
    #endif
  }

  // Get the ABC or XYZ positions in mm
  abce_pos_t target = planner.get_axis_positions_mm();

  #if HAS_DIST_MM_ARG
    const xyze_float_t cart_dist_mm{0};
  #endif

  // Set delta/cartesian axes directly
  target[axis] += distance;                  // The move will be towards the endstop
  planner.buffer_segment(target
    #if HAS_DIST_MM_ARG
      , cart_dist_mm
    #endif
    , real_fr_mm_s, active_extruder
  );

  planner.synchronize(); // wait until z hits endstop

  if (is_moving_towards_endstop) {

    #if HOMING_Z_WITH_PROBE && QUIET_PROBING
      if (axis == Z_AXIS) probe.set_probing_paused(false);
    #endif

    endstops.validate_homing_move();
  }
}

void homeT1Z() {
  const int axis_home_dir = home_dir(Z_AXIS);

  // Homing Z towards the bed? Deploy the Z probe or endstop.
  if (TERN0(HOMING_Z_WITH_PROBE, probe.deploy()))
    return;

  do_t1_homing_move(Z_AXIS, 1.5f * max_length(Z_AXIS) * axis_home_dir);

  // When homing Z with probe respect probe clearance
  const float bump = axis_home_dir * home_bump_mm(Z_AXIS);

  // If a second homing move is configured...
  if (bump) {
    // Move away from the endstop by the axis HOMING_BUMP_MM
    do_t1_homing_move(Z_AXIS, -bump
      #if HOMING_Z_WITH_PROBE
        , MMM_TO_MMS(Z_PROBE_SPEED_FAST)
      #endif
    );

    // Slow move towards endstop until triggered
    do_t1_homing_move(Z_AXIS, 2 * bump, get_homing_bump_feedrate(Z_AXIS));
  }

  // Set T1 Z Offset
  tool_change(0, true); // use t0's z value as the offset
  hotend_offset[1].z = current_position[Z_AXIS];
  SERIAL_ECHO_START();
  SERIAL_ECHOPGM("Set T1 Z Offset to: ");
  SERIAL_ECHO_F(hotend_offset[1].z, 3);
  SERIAL_EOL();
  tool_change(1, true); // change back to t1

  // Put away the Z probe
  #if HOMING_Z_WITH_PROBE
    if (probe.stow()) return;
  #endif

} // homeaxis()

#if ENABLED(Z_SAFE_HOMING)
  inline void home_z_safely() {
    /**
     * Move the Z probe (or just the nozzle) to the safe homing point
     * (Z is already at the right height)
     * Set destination to center of bed with same height
     */
    destination.set(safe_homing_xy, current_position.z);

    TERN_(HOMING_Z_WITH_PROBE, destination -= probe.offset_xy);

    if (position_is_reachable(destination)) {
      // This causes the carriage on Dual X to unpark
      TERN_(DUAL_X_CARRIAGE, active_extruder_parked = false);
      do_blocking_move_to_xy(destination);
      homeT1Z();
    }
    else {
      SERIAL_ECHO_MSG(STR_ZPROBE_OUT_SER);
    }
  }

#endif // Z_SAFE_HOMING

void GcodeSuite::R746() {

  // Add checking for correct state
  // Check T0 already homed
  // Disallow Z homing if X or Y homing is needed
  if (homing_needed_error(_BV(X_AXIS) | _BV(Y_AXIS))) return;
  // Check T1 enabled
  // Check XY position
  // Check dualx mode was not set to something weird

  RAISE_HOMING_SEMAPHORE();

  // Wait for planner moves to finish!
  planner.synchronize();

  hotend_offset[1].z = 0; // reset t1 z offset to 0 before moving

  // Disable the leveling matrix before homing (REVISIT THIS LATER)
  #if HAS_LEVELING

    // Cancel the active G29 session
    TERN_(PROBE_MANUALLY, g29_in_progress = false);

    TERN_(RESTORE_LEVELING_AFTER_G28, const bool leveling_was_active = planner.leveling_active);
    set_bed_leveling_enabled(false);
  #endif

  // Count this command as movement / activity
  reset_stepper_timeout();

  TERN_(HAS_DUPLICATION_MODE, extruder_duplication_enabled = false);

  remember_feedrate_scaling_off();

  endstops.enable(true); // Enable endstops for next homing move

  tool_change(0, true); // change to t0 without moving in order to dock it

  homeaxis(X_AXIS); // always dock t0 to prevent crashing

  tool_change(1, true); // change to t1 without moving

  const float z_homing_height =
    ENABLED(UNKNOWN_Z_NO_RAISE) && !TEST(axis_known_position, Z_AXIS)
      ? 0
      : (parser.seenval('R') ? parser.value_linear_units() : Z_HOMING_HEIGHT);

  if (z_homing_height) {
    // Raise Z before homing any other axes and z is not already high enough (never lower z)
    if (DEBUGGING(LEVELING)) DEBUG_ECHOLNPAIR("Raise Z (before homing) by ", z_homing_height);
    do_z_clearance(z_homing_height, true, DISABLED(UNKNOWN_Z_NO_RAISE));
  }

  // Home Z
  TERN_(BLTOUCH, bltouch.init());

  TERN(Z_SAFE_HOMING, home_z_safely(), homeT1Z());

  probe.move_z_after_homing(); // raise after home

  SERIAL_ECHOLNPGM("Finished homing T1 Z.");

  endstops.not_homing();

  TERN_(RESTORE_LEVELING_AFTER_G28, set_bed_leveling_enabled(leveling_was_active));

  restore_feedrate_and_scaling();

  #if defined(Z_AFTER_HOMING)
    do_blocking_move_to_z(Z_AFTER_HOMING);
  #endif

  report_current_position();
}
