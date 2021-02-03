#include "../../inc/MarlinConfig.h"

#include "../gcode.h"
#include "../../module/motion.h"

#if ENABLED(RAPIDIA_HOMING_RESET)

void GcodeSuite::R745()
{
    set_all_unhomed();
}

#endif // USB_FLASH_DRIVE_SUPPORT

