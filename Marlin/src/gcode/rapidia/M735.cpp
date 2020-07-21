#include "../../inc/MarlinConfig.h"

#include "../gcode.h"
#include "../../module/endstops.h"

#if ENABLED(RAPIDIA_NOZZLE_PLUG_HYSTERESIS)

void GcodeSuite::M735()
{
  if (parser.seenval('S'))
  {
    uint8_t threshold = parser.value_byte();

    if (threshold == 0)
    {
        SERIAL_ECHO_START();
        SERIAL_ECHOLNPGM(
            "Warning: a threshold value of 0 means Z_MAX is always triggered. (Did you mean S1 instead?)"
        );
    }
    
    endstops.z_max_hysteresis_threshold = threshold;
  }

  if (parser.seenval('I'))
  {
    uint16_t interval = parser.value_ushort();
    if (interval != 16)
    {
      SERIAL_ECHO_START();
      SERIAL_ECHOLNPGM(
          "WARNING: interval is hardcoded at 16 ms and cannot be modified."
      );
    }
  }

    // report value.
    SERIAL_ECHO_START();
    SERIAL_ECHOPGM(
    "Z_MAX threshold value is "
    );
    SERIAL_ECHO(endstops.z_max_hysteresis_threshold);
    SERIAL_ECHOPGM(
    "; interval is  "
    );
    SERIAL_ECHO(endstops.z_max_hysteresis_min_interval_ms);
    SERIAL_ECHOLNPGM(" ms.");
}

#if ENABLED(RAPIDIA_NOZZLE_PLUG_HYSTERESIS_DEBUG_RECORDING)
  void GcodeSuite::M734()
  {
    endstops.start_z_max_hysteresis_record();
  }
#endif // ENABLED(RAPIDIA_NOZZLE_PLUG_HYSTERESIS_DEBUG_RECORDING)
#endif

