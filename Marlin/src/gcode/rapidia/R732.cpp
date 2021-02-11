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
      checksum_mode = CHECKSUMS_XOR_OPTIONAL;
      break;
    case 3:
      checksum_mode = CHECKSUMS_CRC16;
      break;
    default:
      SERIAL_ERROR_MSG("Specified checksum mode not recognized.");
      break;
    }
}

static void echo_checksum_name(checksum_mode_t c)
{
  switch(c)
  {
  case CHECKSUMS_DISABLED:
    SERIAL_ECHO("disabled");
    return;
  case CHECKSUMS_XOR:
    SERIAL_ECHO("xor");
    return;
  case CHECKSUMS_XOR_OPTIONAL:
    SERIAL_ECHO("xor (optional)");
    return;
  case CHECKSUMS_CRC16:
    SERIAL_ECHO("crc16/XMODEM");
    return;
  default:
    SERIAL_ECHO("unknown");
    return;
  }
}

void GcodeSuite::R732()
{
  if (parser.seenval('I'))
  {
    set_checksum_mode(checksum_mode_in);
  }

  if (parser.seenval('O'))
  {
    set_checksum_mode(checksum_mode_out);
  }

  // echo input mode
  SERIAL_ECHO_START();
  SERIAL_ECHO("Checksum mode (input): ");
  echo_checksum_name(checksum_mode_in);
  SERIAL_EOL();

  // echo output mode
  SERIAL_ECHO_START();
  SERIAL_ECHO("Checksum mode (output): ");
  echo_checksum_name(checksum_mode_out);
  SERIAL_EOL();
}

#endif // ENABLED(RAPIDIA_HEARTBEAT)
