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

#if ENABLED(RAPIDIA_T1_HOMING)

static void park_toolhead(uint8_t toolhead=active_extruder) {
  uint8_t prev = active_extruder;
  tool_change(toolhead, true);
  do_blocking_move_to_x(x_home_pos(toolhead));
  tool_change(prev, true);
}

static void t1_probe()
{
  tool_change(1, true);

  hotend_offset[1].z = probe.probe_at_point(safe_homing_xy);

  // for z clearance and raising
  tool_change(0, true);

  // probe.move_z_after_homing(); // raise after home
  do_z_clearance(Z_AFTER_HOMING, true, true, false);
}

void GcodeSuite::R746() {

  if (ENABLED(RAPIDIA_PLASTIC))
  {
    SERIAL_ECHO_MSG("R746 not supported on plastic printers");
    return;
  }

  // Check T0 already homed
  if (homing_needed_error()) {
    return;
  }

  // Check dualx mode was not set to something weird
  if (dual_x_carriage_mode != DXC_FULL_CONTROL_MODE) {
    SERIAL_ERROR_MSG("dualx mode must be set to full control mode. (M605 S0)");
    return;
  }

  uint8_t prev_extruder = active_extruder;

  RAISE_HOMING_SEMAPHORE();

  // Wait for planner moves to finish.
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

  endstops.enable(true); // Enable endstops upcoming homing moves

  park_toolhead(0); // always dock t0 to prevent crashing

  TERN_(BLTOUCH, bltouch.init());

  // main homing logic for T1.
  t1_probe();

  // restore previous settings
  endstops.not_homing();
  TERN_(RESTORE_LEVELING_AFTER_G28, set_bed_leveling_enabled(leveling_was_active));
  tool_change(prev_extruder, true);
  restore_feedrate_and_scaling();

  report_current_position();
}

#endif