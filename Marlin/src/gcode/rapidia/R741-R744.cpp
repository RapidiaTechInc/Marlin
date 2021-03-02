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
    const double amount_mm = parser.seenval('V') ? parser.value_float() : 0;

    // can't store negative mileage.
    if (amount_mm < 0)
    {
        SERIAL_ERROR_MSG("Cannot store negative mileage value.");
        return;
    }

    const uint64_t amount_um = parser.seenval('U')
        ? parser.value_ulong64()
        : static_cast<uint64_t>(amount_mm * 1000);

    const uint64_t amount_nm = parser.seenval('N')
        ? parser.value_ulong64()
        : (amount_um * 1000);
    
    const bool save = parser.seenval('S') ? parser.value_bool() : true;

    MileageData& data = mileage.data();

    if (extruder == 0)
    {
        for (size_t i = 0; i < EXTRUDERS; ++i)
        {
            data.e_mm[i] = Rapidia::u64nm_to_mileage(amount_nm);
        }
    }
    else
    {
        if (extruder - 1 >= EXTRUDERS)
        {
            SERIAL_ERROR_MSG("invalid extruder number.");
            return;
        }
        data.e_mm[extruder - 1] = Rapidia::u64nm_to_mileage(amount_nm);
    }

    if (save && mileage.save_eeprom())
    {
        SERIAL_ERROR_MSG("Mileage was not saved to EEPROM.");
    }
}

#endif // USB_FLASH_DRIVE_SUPPORT

