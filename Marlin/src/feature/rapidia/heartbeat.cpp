#include "heartbeat.h"

#include "../../inc/MarlinConfig.h"
#include "../../module/stepper.h"
#include "../../module/planner.h"
#include "../../gcode/gcode.h"

#if ENABLED(RAPIDIA_HEARTBEAT)

namespace Rapidia
{
static uint16_t heartbeat_interval = 0;
static millis_t next_heartbeat_report_ms = 0;
Heartbeat heartbeat; // singleton

HeartbeatSelectionUint Heartbeat::selection
  = (HeartbeatSelectionUint)HeartbeatSelection::_DEFAULT;

void Heartbeat::set_interval(uint16_t v)
{
  heartbeat_interval = v;
  next_heartbeat_report_ms = millis() + v;
}

static void report_xyzetf(const xyze_pos_t &pos, const uint8_t extruder, const bool feedrate=false, const uint8_t n=XYZE, const uint8_t precision=3) {
  char str[12];
  
  // position.
  LOOP_L_N(a, n) {
    SERIAL_CHAR(axis_codes[a], ':');
    SERIAL_ECHO(dtostrf(pos[a], 1, precision, str));
    SERIAL_CHAR(',');
  }
  
  if (feedrate)
  {
    SERIAL_CHAR('F', ':');
    SERIAL_ECHO(dtostrf(feedrate_mm_s, 1, precision, str));
    SERIAL_CHAR(',');
  }
  
  // extruder number
  SERIAL_CHAR('T', ':');
  SERIAL_CHAR('0' + extruder);
}

#define TEST_FLAG(a, b) (!!((uint32_t)(a) & (uint32_t)(b)))

void Heartbeat::serial_info(HeartbeatSelection selection, bool bare)
{
  // begin heartbeat
  if (!bare)
  {
    SERIAL_CHAR(' ');
    SERIAL_CHAR('H');
    SERIAL_CHAR(':');
    SERIAL_CHAR('{');
  }
  
  // plan position
  if (TEST_FLAG(selection, HeartbeatSelection::PLAN_POSITION))
  {
    SERIAL_CHAR('P');
    SERIAL_CHAR(':');
    SERIAL_CHAR('{');
    
    report_xyzetf(current_position.asLogical(), active_extruder, TEST_FLAG(selection, HeartbeatSelection::FEEDRATE));
    SERIAL_CHAR('}');
    SERIAL_CHAR(',');
  }
  
  // actual position
  if (TEST_FLAG(selection, HeartbeatSelection::ABS_POSITION))
  {
    SERIAL_CHAR('C');
    SERIAL_CHAR(':');
    SERIAL_CHAR('{');
    
    // unit conversion steps -> logical
    Stepper::State state = stepper.report_state();
    xyze_pos_t position;
    LOOP_XYZE(axis)
    {
      position[axis] = state.position[axis] / planner.settings.axis_steps_per_mm[axis];
    }
  
    report_xyzetf(position.asLogical(), state.extruder);
    SERIAL_CHAR('}');
    SERIAL_CHAR(',');
  }
  
  // relative mode axes:
  if (TEST_FLAG(selection, HeartbeatSelection::RELMODE))
  {
    SERIAL_CHAR('R');
    SERIAL_CHAR(':');
    SERIAL_CHAR('"');
    LOOP_XYZE(axis)
    {
      if (gcode.axis_is_relative(AxisEnum(axis)))
      {
        SERIAL_CHAR(axis_codes[axis]);
      }
    }
    SERIAL_CHAR('"');
    SERIAL_CHAR(',');
  }
  
  // dualx info
  if (TEST_FLAG(selection, HeartbeatSelection::DUALX))
  {
    SERIAL_CHAR('X', ':');
    SERIAL_CHAR('{');
    {
      char str[12];
      
      // dual_x_carriage_mode
      SERIAL_CHAR('S', ':');
      SERIAL_CHAR('0' + (int32_t)(dual_x_carriage_mode));
      SERIAL_CHAR(',');
      
      // active toolhead
      SERIAL_CHAR('T', ':');
      SERIAL_CHAR('0' + (int32_t)(active_extruder));
      SERIAL_CHAR(',');
      
      // stored x position
      SERIAL_CHAR('X', ':');
      SERIAL_ECHO(dtostrf(inactive_extruder_x_pos, 1, 3, str));
      
      // TODO: stored feedrate.
    }
    SERIAL_CHAR('}');
    SERIAL_CHAR(',');
  }
  
  // dummy data at end of json so that we don't have to worry about separators.
  if (!bare)
  {
    SERIAL_CHAR('_');
    SERIAL_CHAR(':');
    SERIAL_CHAR('0');
    
    SERIAL_CHAR('}');
  }
  // end heartbeat
}

void Heartbeat::auto_report()
{
  if (heartbeat_interval && ELAPSED(millis(), next_heartbeat_report_ms)) {
    next_heartbeat_report_ms = millis() + heartbeat_interval;

    PORT_REDIRECT(SERIAL_BOTH);
    serial_info(static_cast<HeartbeatSelection>(Heartbeat::selection));
    SERIAL_EOL();
  }
}

void Heartbeat::select(HeartbeatSelection selection, bool enable)
{
  if (enable)
  {
    Heartbeat::selection |= static_cast<HeartbeatSelectionUint>(selection);
  }
  else
  {
    Heartbeat::selection &= ~static_cast<HeartbeatSelectionUint>(selection);
  }
}

} // namespace Rapidia

#endif // ENABLED(RAPIDIA_HEARTBEAT)