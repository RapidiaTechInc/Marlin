#include "pause.h"

#include "../../module/planner.h"
#include "../../module/stepper.h"
#include "../../gcode/queue.h"
#include "../../sd/cardreader.h"
#include "heartbeat.h"

#if ENABLED(RAPIDIA_PAUSE)
namespace Rapidia
{
Pause pause; // singleton

static uint8_t defer_pause = 0;

static void report_xyzet(const xyze_pos_t &pos, const uint8_t extruder, const uint8_t n=XYZE, const uint8_t precision=3) {
  char str[12];
  
  // position.
  LOOP_L_N(a, n) {
    SERIAL_CHAR(axis_codes[a], ':');
    SERIAL_ECHO(dtostrf(pos[a], 1, precision, str));
    SERIAL_CHAR(',');
  }
  
  // extruder number
  SERIAL_CHAR('T', ':');
  SERIAL_CHAR('0' + extruder);
}

void Pause::pause(bool hard)
{
  Stepper::State pause_state = stepper.report_state();
  
  long qline = queue.get_first_line_number();
  
  // pause SD card printing
  #if ENABLED(SDSUPPORT)
    static bool was_printing_sd = false;
    if (IS_SD_PRINTING())
    {
      was_printing_sd = true;
      card.pauseSDPrint();
    }
  #endif
  
  // process no further commands.
  queue.clear();
  
  // tell planner to pause.
  Planner::pause_result result = planner.pause_decelerate(hard);
  
  if (result.defer)
  {
    // try to pause again next idle loop.
    defer_pause = hard + 1;
    return;
  }
  
  // cancel any gcode higher-up on the callstack.
  planner.prevent_block_buffering = true;
  
  // Wait for the pause to complete.
  planner.synchronize();
  
  // new plan position = calculated position from stepper.
  set_current_from_steppers_for_axis(ALL_AXES);
  sync_plan_position();
  
  // report pause result.
  SERIAL_ECHO("pause:{");
  if (qline != -1)
  {
    SERIAL_ECHO("N:");
    SERIAL_ECHO(qline);
    SERIAL_CHAR(',');
  }
  if (result.line >= 0)
  {
    SERIAL_ECHO("G:");
    SERIAL_ECHO(result.line);
    SERIAL_CHAR(',');
  }
  if (!result.deceleration_block)
  {
    SERIAL_ECHO("deceleration:false");
    if (result.deceleration_cropped)
    {
      SERIAL_ECHO(",cropped:false");
    }
    else
    {
      SERIAL_ECHO(",cropped:true");
    }
    SERIAL_CHAR(',');
  }
  else
  {
    SERIAL_ECHO("deceleration:true,");
    SERIAL_ECHO("distance:");
    SERIAL_ECHO(result.deceleration_mm);
    SERIAL_CHAR(',');
  }
  #if ENABLED(SDSUPPORT)
    SERIAL_ECHO("sd:");
    if (was_printing_sd)
    {
      SERIAL_ECHO("true");
    }
    else
    {
      SERIAL_ECHO("false");
    }
    SERIAL_CHAR(',');
    was_printing_sd = false;
  #endif
  
  // pre-pause position
  SERIAL_CHAR('P');
  SERIAL_CHAR(':');
  SERIAL_CHAR('{');
  
  xyze_pos_t position;
  LOOP_XYZE(axis)
  {
    position[axis] = pause_state.position[axis] / planner.settings.axis_steps_per_mm[axis];
  }

  report_xyzet(position.asLogical(), pause_state.extruder);
  SERIAL_CHAR('}', ',');
  
  // position
  SERIAL_CHAR('C');
  SERIAL_CHAR(':');
  SERIAL_CHAR('{');
  report_xyzet(current_position.asLogical(), active_extruder);
  SERIAL_CHAR('}');
  
  // end of message
  SERIAL_ECHOLN("}");
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