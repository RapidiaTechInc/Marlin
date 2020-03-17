#include "../../inc/MarlinConfig.h"

#include "../gcode.h"

#include "../../core/debug_out.h"

#if ENABLED(CONDITIONAL_GCODE)
unsigned long timers_m710[M710_TIMER_COUNT];
uint8_t skipGCode;

void M710()
{
    uint8_t timerIndex = 0;
    if (parser.seenval('T'))
    {
        timerIndex = parser.value_byte();
    }
    
    if (timerIndex >= M710_TIMER_COUNT)
    {
        SERIAL_ECHO_START();
        SERIAL_ECHOLN(MSG_INVALID_INDEX);
    }
    else
    {
        timers_m710[timerIndex] = millis();
        SERIAL_ECHO_START();
        SERIAL_ECHO("Set timer ");
        SERIAL_PROTOCOLLN(timerIndex);
    }
}

void M711()
{
    uint8_t timerIndex = 0;
    unsigned long comp = 1000;
    bool above = 0;
    
    if (parser.seenval('T'))
    {
        timerIndex = parser.value_byte();
    }
    
    if (parser.seenval('A'))
    {
        comp = parser.value_ulong();
        above = true;
    }
    if (parser.seenval('B'))
    {
        comp = parser.value_ulong();
        above = false;
    }
    
    // note that even though millis() can overflow, it doesn't matter here.
    long timerval = static_cast<long>(millis()) - static_cast<long>(timers_m710[timerIndex]);
    
    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("Timer comparison. ");
    SERIAL_PROTOCOL(timerval);
    if (above)
    {
        SERIAL_ECHOPGM(" >= ");
    }
    else
    {
        SERIAL_ECHOPGM(" < ");
    }
    SERIAL_PROTOCOL(comp);
    SERIAL_ECHOLNPGM("?");
    
    if ((timerval >= comp) != above)
    {
        // condition failed; skip the next N lines of gcode.
        uint8_t lines = 1;
        if (parser.seenval('L'))
        {
            lines = comp = parser.value_byte();
        }
        
        SERIAL_ECHOPGM("No. Skipping ");
        SERIAL_PROTOCOL(lines);
        SERIAL_ECHOLNPGM(" lines of gcode.");
        
        if (skipGCode < lines)
        {
            skipGCode = lines;
        }
    }
    else
    {
        SERIAL_ECHOLNPGM("Yes. No GCode will be skipped.");
    }
}

#endif // CONDITIONAL_GCODE

