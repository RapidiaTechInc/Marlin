#include "../../../inc/MarlinConfig.h"
#include "../../gcode.h"

#if ENABLED(RAPIDIA_DEV_CODES)

static volatile bool softlock = false;

void GcodeSuite::R800()
{
    const bool do_cli = (parser.seenval('I') ? parser.value_byte() : 0);

    if (do_cli)
    {
        cli();
    }

    softlock = true;
    while (softlock);
    
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

