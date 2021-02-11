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

#if ENABLED(RAPIDIA_CHECKSUMS)

#include "../gcode.h"
#include "../../feature/rapidia/checksum.h"

using namespace Rapidia;

static void set_checksum_mode(checksum_mode_t& checksum_mode)
{
  auto val = parser.value_ushort();
    switch (val)
    {
    case 0:
      checksum_mode = CHECKSUMS_DISABLED;
      break;
    case 1:
      checksum_mode = CHECKSUMS_XOR;
      break;
    case 2:
      checksum_mode = CHECKSUMS_CRC16;
      break;
    default:
      SERIAL_ERROR_MSG("Specified checksum mode not recognized.");
      break;
    }
}

void GcodeSuite::R732()
{
  if (parser.seenval('I'))
  {
    SERIAL_ERROR_MSG("Cannot set input checksum mode on this firmware version.");
  }

  if (parser.seenval('O'))
  {
    set_checksum_mode(checksum_mode_out);
  }
}

#endif // ENABLED(RAPIDIA_HEARTBEAT)
