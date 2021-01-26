#include "../../inc/MarlinConfig.h"

#include "../gcode.h"
#include "../../sd/SdVolume.h"
#include "../../MarlinCore.h"

#if ENABLED(USB_FLASH_DRIVE_SUPPORT)

void GcodeSuite::R740()
{
    Sd2Card::usbStartup();

    // delay for 500 ms to allow sd card setup
    // (done in idle()).
    const millis_t time = millis() + 500;
    while (PENDING(millis(), time)) idle();
}

#endif // USB_FLASH_DRIVE_SUPPORT

