#include "../../inc/MarlinConfig.h"

#include "../gcode.h"
#include "../../feature/rapidia/pause.h"

#if ENABLED(RAPIDIA_PAUSE)
#if DISABLED(EMERGENCY_PARSER)

void GcodeSuite::M751()
{
    Rapidia::pause.pause(false);
}

void GcodeSuite::M752()
{
    Rapidia::pause.pause(true);
}

#endif // EmergencyParser
#endif // CONDITIONAL_GCODE

