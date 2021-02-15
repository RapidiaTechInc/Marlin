#include "../../../inc/MarlinConfig.h"
#include "../../../feature/rapidia/stack_util.h"
#include "../../gcode.h"
#include "../../../HAL/HAL.h"
#include <stdlib.h>

#if ENABLED(RAPIDIA_DEV)

#include <setjmp.h>

static volatile bool softlock = false;

using namespace Rapidia;

void GcodeSuite::R802()
{
    cli();
    uint8_t lowBits      = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    uint8_t highBits     = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    uint8_t extendedBits = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
    uint8_t lockBits     = boot_lock_fuse_bits_get(GET_LOCK_BITS);
    sei();

    char sbuff[80];

    #ifdef __GNUC__
    SERIAL_ECHO_START();
    sprintf(sbuff, "gcc %u.%u.%u\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    SERIAL_ECHO(sbuff);
    #endif

    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("fuse bits: ");
    sprintf_P(sbuff, PSTR("low:%2hhx, high:%2hhx, ext:%2hhx lock:%2hhx\n"),
        lowBits, highBits, extendedBits, lockBits
    );
    SERIAL_ECHO(sbuff);

    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("heap space:");
    sprintf_P(sbuff, PSTR("start:%p end:%p (%u bytes)\n"),
        __malloc_heap_start, __malloc_heap_end, 
        (__malloc_heap_end < __malloc_heap_start)
            ? __malloc_heap_start - __malloc_heap_end
            : __malloc_heap_end - __malloc_heap_start
    );
    SERIAL_ECHO(sbuff);

    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("stack space:");
    sprintf_P(sbuff, PSTR("start:%p end:%p (%hu bytes)\n"),
        avr_stack_start, avr_stack_end,
        avr_stack_size
    );
    SERIAL_ECHO(sbuff);

    #ifdef RAPIDIA_STACK_USAGE
    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("stack usage (sup):");
    sprintf_P(sbuff, PSTR("%hu\n"), calc_stack_usage());
    SERIAL_ECHO(sbuff);
    #endif

    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("current stack pointer:");
    sprintf_P(sbuff, PSTR("%p\n"), get_stack_top());
    SERIAL_ECHO(sbuff);

    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("current stack depth:");
    sprintf_P(sbuff, PSTR("%hu\n"), get_stack_depth());
    SERIAL_ECHO(sbuff);

    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("architecture context size:");
    sprintf_P(sbuff, PSTR("%u\n"), static_cast<unsigned int>(sizeof(jmp_buf)));
    SERIAL_ECHO(sbuff);
}
#endif // RAPIDIA_DEV

