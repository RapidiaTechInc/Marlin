#include "../../inc/MarlinConfig.h"

#include "../gcode.h"
#include "../../feature/rapidia/mileage.h"

#if ENABLED(RAPIDIA_MILEAGE)

using namespace Rapidia;

// reset mileage
void GcodeSuite::R741()
{
    SERIAL_ECHO_MSG("Mileage data reset.");
    mileage.reset();
}

// save mileage immediately
void GcodeSuite::R742()
{
    if (mileage.save_eeprom())
    {
        SERIAL_ERROR_MSG("Mileage was not saved.");
    }
    else
    {
        SERIAL_ECHO_MSG("Mileage saved successfully.");
    }
}

// set mileage save interval
void GcodeSuite::R743()
{
    if (parser.seenval('I'))
    {
        uint32_t interval = parser.value_ushort();
        mileage.save_interval_ms = SEC_TO_MS(interval);
    }

    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("Maximum mileage save interval:");
    SERIAL_ECHO(mileage.save_interval_ms);
    SERIAL_ECHOLNPGM(" ms.");
}

void GcodeSuite::R744()
{
    // modify mileage
    const uint8_t extruder = parser.seenval('E') ? parser.value_byte() : 0; // 0 = all extruders
    const double amount = parser.seenval('V') ? parser.value_float() : 0;
    const bool save = parser.seenval('S') ? parser.value_bool() : true;

    MileageData& data = mileage.data();

    if (extruder == 0)
    {
        for (size_t i = 0; i < EXTRUDERS; ++i)
        {
            if (amount < 0) {
              data.e_mm[i] = 0;
            } else {
              data.e_mm[i] = amount;
            }
            SERIAL_ECHO_START();
            SERIAL_ECHOPGM("E");
            SERIAL_ECHO(i);
            SERIAL_ECHOPGM(" set to ");
            SERIAL_ECHO(data.e_mm[i]);
            SERIAL_EOL();
        }
    }
    else
    {
        if (extruder - 1 >= EXTRUDERS)
        {
            SERIAL_ERROR_MSG("invalid extruder number.");
            return;
        }
        if (amount < 0) {
          data.e_mm[extruder - 1] = 0;
        } else {
          data.e_mm[extruder - 1] = amount;
        }
        SERIAL_ECHO_START();
        SERIAL_ECHOPGM("E");
        SERIAL_ECHO(extruder - 1);
        SERIAL_ECHOPGM(" set to ");
        SERIAL_ECHO(data.e_mm[extruder - 1]);
        SERIAL_EOL();
    }

    if (save && mileage.save_eeprom())
    {
        SERIAL_ERROR_MSG("Mileage was not saved to EEPROM.");
    }
}

#endif // USB_FLASH_DRIVE_SUPPORT

