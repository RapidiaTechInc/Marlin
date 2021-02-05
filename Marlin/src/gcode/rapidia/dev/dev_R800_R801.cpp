#include "../../../inc/MarlinConfig.h"
#include "../../gcode.h"

#if ENABLED(RAPIDIA_DEV_CODES)

static volatile bool softlock = false;

void GcodeSuite::R800()
{
    const bool do_cli = (parser.seenval('I') ? parser.value_byte() : 0);
    const bool do_wdr = (parser.seenval('W') ? parser.value_byte() : 0);

    const char* p = parser.string_arg;
    for (int i = 0; i < 2; ++i)
    {
        if (p[0] == 'I') p += 2;
        if (p[0] == 'W') p += 2;
        while (p[0] == ' ') ++p;
    }

    if (do_cli)
    {
        cli();
    }

    softlock = true;
    while (softlock)
    {
        if (p[0])
        {
            SERIAL_ECHOLN(p);
        }

        if (do_wdr)
        {
            watchdog_refresh();
        }
    }
    
    if (!do_cli)
    {
        sei();
    }
}

namespace Rapidia
{
void R801()
{
    softlock = false;
}
}
#endif // RAPIDIA_DEV_CODES

