#include "heartbeat.h"

#include "../../inc/MarlinConfig.h"
#include "../../module/stepper.h"
#include "../../module/planner.h"
#include "../../module/endstops.h"
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
    SERIAL_ECHOPGM(" H:{");
  }
  
  // plan position
  if (TEST_FLAG(selection, HeartbeatSelection::PLAN_POSITION))
  {
    SERIAL_ECHOPGM("P:{");
    
    report_xyzetf(current_position.asLogical(), active_extruder, TEST_FLAG(selection, HeartbeatSelection::FEEDRATE));
    SERIAL_CHAR('}', ',');
  }
  
  // actual position
  if (TEST_FLAG(selection, HeartbeatSelection::ABS_POSITION))
  {
    SERIAL_ECHOPGM("C:{");
    
    // unit conversion steps -> logical
    Stepper::State state = stepper.report_state();
    xyze_pos_t position;
    LOOP_XYZE(axis)
    {
      position[axis] = state.position[axis] / planner.settings.axis_steps_per_mm[axis];
    }
  
    report_xyzetf(position.asLogical(), state.extruder);
    SERIAL_CHAR('}', ',');
  }
  
  // relative mode axes:
  if (TEST_FLAG(selection, HeartbeatSelection::RELMODE))
  {
   SERIAL_ECHOPGM("R:\"");
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
    SERIAL_ECHOPGM("X:{");
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
    SERIAL_CHAR('}', ',');
  }
  
  // endstops
  if (TEST_FLAG(selection, HeartbeatSelection::ENDSTOPS))
  {
    // report endstops closed state (at this moment)
    SERIAL_ECHOPGM("E:\"");
    
    // read from the endstop pins directly.
    // (this info doesn't seem to be cached in the Endstops class.)
    
    #define ES_TRIGGERED(S) READ(S##_PIN) != S##_ENDSTOP_INVERTING
    #define ES_REPORT(S, N) if (ES_TRIGGERED(S)) SERIAL_CHAR(N);
    
    #if HAS_X_MIN
      ES_REPORT(X_MIN, 'x');
    #endif
    #if HAS_X_MAX
      ES_REPORT(X_MAX, 'X');
    #endif
    #if HAS_Y_MIN
      ES_REPORT(Y_MIN, 'y');
    #endif
    #if HAS_Y_MAX
      ES_REPORT(Y_MAX, 'Y');
    #endif
    #if HAS_Z_MIN
      ES_REPORT(Z_MIN, 'z');
    #endif
    #if HAS_Z_MAX
      ES_REPORT(Z_MAX, 'Z');
    #endif
   
    SERIAL_CHAR('"', ',');
  }
  
  // dummy data at end of json so that we don't have to worry about separators.
  if (!bare)
  {
    SERIAL_ECHOPGM("_:0}");
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