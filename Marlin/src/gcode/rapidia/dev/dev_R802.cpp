#include "../../../inc/MarlinConfig.h"
#include "../../gcode.h"
#include "avr/boot.h"

#if ENABLED(RAPIDIA_DEV_CODES)

static volatile bool softlock = false;

void GcodeSuite::R802()
{
    cli();
    uint8_t lowBits      = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    uint8_t highBits     = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    uint8_t extendedBits = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
    uint8_t lockBits     = boot_lock_fuse_bits_get(GET_LOCK_BITS);
    sei();

    char sbuff[32];

    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("fuse bits:");
    sprintf(sbuff, "%x %x, %x %x\n", lowBits, highBits, extendedBits, lockBits);
    SERIAL_ECHO(sbuff);
}
#endif // RAPIDIA_DEV_CODES

