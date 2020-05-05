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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(AUTO_REPORT_TEMPERATURES) && HAS_TEMP_SENSOR

#include "../gcode.h"
#include "../../module/temperature.h"
#include "../../feature/rapidia/heartbeat.h"

/**
 * M155: Set temperature auto-report interval. M155 S<seconds>
 */
void GcodeSuite::M155() {

  if (parser.seenval('S'))
  {
    uint8_t interval = parser.value_byte();
    
    thermalManager.set_auto_report_interval(interval);
  }
  
  #if ENABLED(RAPIDIA_HEARTBEAT)
  using namespace Rapidia;
  
  if (parser.seenval('H'))
  {
    uint16_t interval = parser.value_ushort();
    heartbeat.set_interval(interval);
  }
  
  if (parser.seenval('P'))
  {
    uint16_t enabled = parser.value_ushort();
    heartbeat.select(HeartbeatSelection::PLAN_POSITION, enabled);
  }
  
  if (parser.seenval('C'))
  {
    uint16_t enabled = parser.value_ushort();
    heartbeat.select(HeartbeatSelection::ABS_POSITION, enabled);
  }
  
  if (parser.seenval('R'))
  {
    uint16_t enabled = parser.value_ushort();
    heartbeat.select(HeartbeatSelection::RELMODE, enabled);
  }
  
  if (parser.seenval('F'))
  {
    uint16_t enabled = parser.value_ushort();
    heartbeat.select(HeartbeatSelection::FEEDRATE, enabled);
  }
  
  if (parser.seenval('X'))
  {
    uint16_t enabled = parser.value_ushort();
    heartbeat.select(HeartbeatSelection::DUALX, enabled);
  }
  
  if (parser.seenval('E'))
  {
    uint16_t enabled = parser.value_ushort();
    heartbeat.select(HeartbeatSelection::ENDSTOPS, enabled);
  }
  #endif
}

#endif // AUTO_REPORT_TEMPERATURES && HAS_TEMP_SENSOR
