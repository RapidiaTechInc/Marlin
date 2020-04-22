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
  
  SERIAL_ECHO_START();
  SERIAL_ECHOLNPGM("Pausing!");
  
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
  static bool pause_in_progress = false;
  if (pause_in_progress) return;
  pause_in_progress = true;
  switch (defer_pause)
  {
  case 1: // soft
    defer_pause=0;
    pause(false);
    break;
    
  case 2: // hard
    defer_pause=0;
    pause(true);
    break;
    
  default:
    break;
  }
  pause_in_progress = false;
}

}
#endif