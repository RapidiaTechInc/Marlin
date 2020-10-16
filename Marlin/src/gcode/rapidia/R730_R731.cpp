#include "../../inc/MarlinConfig.h"

#include "../gcode.h"
#include "../../module/planner.h"

#include "../../core/debug_out.h"

#if ENABLED(RAPIDIA_LINE_AUTO_REPORTING)

void GcodeSuite::R730()
{
    planner.auto_report_line_finished = true;
}

void GcodeSuite::R731()
{
    planner.auto_report_line_finished = false;
}
#endif // CONDITIONAL_GCODE

