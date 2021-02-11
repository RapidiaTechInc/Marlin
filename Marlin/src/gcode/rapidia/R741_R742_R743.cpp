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

#endif // USB_FLASH_DRIVE_SUPPORT

