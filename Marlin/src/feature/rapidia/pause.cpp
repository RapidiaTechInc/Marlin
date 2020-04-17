#include "pause.h"

#include "../../module/planner.h"
#include "../../module/stepper.h"

#if ENABLED(RAPIDIA_HEARTBEAT)
namespace Rapidia
{
Pause pause; // singleton

static uint8_t defer_pause = 0;

void Pause::pause(bool hard)
{
  // hard yet-to-be-implemented.
  hard = false;
  
  source_line_t line = planner.pause_decelerate();
  planner.synchronize();
  
  // new plan position = calculated position from stepper.  
  set_current_from_steppers_for_axis(ALL_AXES);
  sync_plan_position();
}

void Pause::defer(bool hard)
{
  defer_pause = hard + 1;
}

void Pause::process_deferred()
{
  switch (defer_pause)
  {
  case 1: // soft
    pause(false);
    defer_pause=0;
    break;
    
  case 2: // hard
    pause(true);
    defer_pause=0;
    break;
    
  default:
    break;
  }
}

}
#endif