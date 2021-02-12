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

#if ENABLED(RAPIDIA_HEARTBEAT)

#include "../gcode.h"
#include "../../feature/rapidia/heartbeat.h"

using namespace Rapidia;

void apply_select(HeartbeatSelectionUint& io_selection, HeartbeatSelection apply_selection, bool enable)
{
  if (enable)
  {
    io_selection |= static_cast<HeartbeatSelectionUint>(apply_selection);
  }
  else
  {
    io_selection &= ~static_cast<HeartbeatSelectionUint>(apply_selection);
  }
}

static void parse_heartbeat_select(HeartbeatSelectionUint& io_heartbeat_select)
{
if (parser.seenval('P'))
  {
    uint16_t enabled = parser.value_ushort();
    apply_select(io_heartbeat_select, HeartbeatSelection::PLAN_POSITION, enabled);
  }
  
  if (parser.seenval('C'))
  {
    uint16_t enabled = parser.value_ushort();
    apply_select(io_heartbeat_select, HeartbeatSelection::ABS_POSITION, enabled);
  }
  
  if (parser.seenval('R'))
  {
    uint16_t enabled = parser.value_ushort();
    apply_select(io_heartbeat_select, HeartbeatSelection::RELMODE, enabled);
  }
  
  if (parser.seenval('F'))
  {
    uint16_t enabled = parser.value_ushort();
    apply_select(io_heartbeat_select, HeartbeatSelection::FEEDRATE, enabled);
  }
  
  if (parser.seenval('X'))
  {
    uint16_t enabled = parser.value_ushort();
    apply_select(io_heartbeat_select, HeartbeatSelection::DUALX, enabled);
  }
  
  if (parser.seenval('E'))
  {
    uint16_t enabled = parser.value_ushort();
    apply_select(io_heartbeat_select, HeartbeatSelection::ENDSTOPS, enabled);
  }

  if (parser.seenval('M'))
  {
    uint16_t enabled = parser.value_ushort();
    apply_select(io_heartbeat_select, HeartbeatSelection::MILEAGE, enabled);
  }

  if (parser.seenval('D'))
  {
    uint16_t enabled = parser.value_ushort();
    apply_select(io_heartbeat_select, HeartbeatSelection::DEBUG, enabled);
  }
}

void GcodeSuite::R738()
{
  if (parser.seenval('H'))
  {
    uint16_t interval = parser.value_ushort();
    heartbeat.set_interval(interval);
  }

  parse_heartbeat_select(heartbeat.selection);
}

void GcodeSuite::R739()
{
  HeartbeatSelectionUint selection = heartbeat.selection;

  if (parser.seenval('A'))
  {
    uint16_t enabled = parser.value_ushort();
    if (enabled == 0)
    {
      selection = 0;
    }
    else if (enabled == 1)
    {
      selection = static_cast<HeartbeatSelectionUint>(HeartbeatSelection::_DEFAULT);
    }
    else
    {
      selection = static_cast<HeartbeatSelectionUint>(HeartbeatSelection::_ALL);
    }
  }

  parse_heartbeat_select(selection);
  heartbeat.serial_info(selection);
}

#endif // ENABLED(RAPIDIA_HEARTBEAT)
