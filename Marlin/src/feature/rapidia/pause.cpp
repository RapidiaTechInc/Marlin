#include "pause.h"

#include "../../module/planner.h"
#include "../../module/stepper.h"
#include "../../gcode/queue.h"
#include "../../sd/cardreader.h"
#include "../../gcode/gcode.h"
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
    echo_key(axis_codes[a]);
    SERIAL_ECHO(dtostrf(pos[a], 1, precision, str));
    SERIAL_CHAR(',');
  }
  
  // extruder number
  echo_key('T');
  SERIAL_CHAR('0' + extruder);
}

void Pause::pause(bool hard)
{
  // prevent re-entry
  static bool is_pausing = false;
  if (is_pausing) return;
  is_pausing = true;

  // what command is being interrupted
  char command_letter = GcodeSuite::dbg_current_command_letter;
  int codenum = GcodeSuite::dbg_current_codenum;

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
    is_pausing = false;
    return;
  }
  
  // cancel any gcode higher-up on the callstack.
  planner.prevent_block_buffering = true;
  
  // Wait for the toolhead to decelerate and come to a complete rest.
  planner.synchronize();
  
  // new plan position = calculated position from stepper.
  set_current_from_steppers_for_axis(ALL_AXES);
  sync_plan_position();
  
  // report pause result.
  SERIAL_ECHO("pause:{");

  // separator accumulator
  bool sep = true;

  // line number to report?
  if (qline != -1)
  {
    echo_separator(sep);
    echo_key('N');
    SERIAL_ECHO(qline);
  }

  // report gcode line that was completed?
  if (result.line >= 0)
  {
    echo_separator(sep);
    echo_key('G');
    SERIAL_ECHO(result.line);
  }

  // report command that was interrupted by pause?
  if (command_letter)
  {
    char sbuff[32];
    if (sprintf(sbuff, "\"%c%d\"", command_letter, codenum) > 0)
    {
      echo_separator(sep);
      echo_key('I');
      SERIAL_ECHO(sbuff);
    }
  }

  // did deceleration occur?
  echo_separator(sep);
  echo_key_str("deceleration");
  if (!result.deceleration_block)
  {
    SERIAL_ECHO("false");

    // for debugging purposes
    echo_separator(sep);
    echo_key_str("cropped");
    if (result.deceleration_cropped)
    {
      SERIAL_ECHO("false");
    }
    else
    {
      SERIAL_ECHO("true");
    }
  }
  else
  {
    SERIAL_ECHO("true");

    // distance travelled during deceleration.
    echo_separator(sep);
    echo_key_str("distance");
    SERIAL_ECHO(result.deceleration_mm);
  }

  #if ENABLED(SDSUPPORT)
    // report if the pause occured during an SD print.
    // (if true, this means the sd print was auto-paused.)
    echo_separator(sep);
    echo_key_str("sd");
    if (was_printing_sd)
    {
      SERIAL_ECHO("true");
    }
    else
    {
      SERIAL_ECHO("false");
    }
    was_printing_sd = false;
  #endif
  
  // report the position from immediately before pausing.
  echo_separator(sep);
  echo_key('P');
  SERIAL_CHAR('{');
  
  xyze_pos_t position;
  LOOP_XYZE(axis)
  {
    position[axis] = pause_state.position[axis] / planner.settings.axis_steps_per_mm[axis];
  }

  report_xyzet(position.asLogical(), pause_state.extruder);
  SERIAL_CHAR('}');
  
  // current position after pausing
  echo_separator(sep);
  echo_key('C');
  SERIAL_CHAR('{');
  report_xyzet(current_position.asLogical(), active_extruder);
  SERIAL_CHAR('}');
  
  // end of message
  SERIAL_ECHOLN("}");

  // allow entry.
  is_pausing = false;
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