#include "heartbeat.h"

#include "../inc/MarlinConfig.h"
#include "../module/stepper.h"
#include "../module/planner.h"
#include "../gcode/gcode.h"

#if ENABLED(RAPIDIA_HEARTBEAT)

namespace Rapidia
{
static uint16_t heartbeat_interval = 0;
static millis_t next_heartbeat_report_ms = 0;
HeartbeatSelection Heartbeat::selection =  HeartbeatSelection::ALL;

void Heartbeat::set_interval(uint16_t v)
{
  heartbeat_interval = v;
  next_heartbeat_report_ms = millis() + v;
}

// lifted wholesale from M114.cpp
static void report_xyze(const xyze_pos_t &pos, const uint8_t n=XYZE, const uint8_t precision=3) {
  char str[12];
  bool first = true;
  LOOP_L_N(a, n) {
    if (!first) SERIAL_CHAR(',');
    SERIAL_CHAR(axis_codes[a], ':');
    SERIAL_ECHO(dtostrf(pos[a], 1, precision, str));
    first = false;
  }
}

#define TEST_FLAG(a, b) (!!((uint32_t)(a) & (uint32_t)(b)))

void Heartbeat::serial_info(HeartbeatSelection selection)
{
  // begin heartbeat
  SERIAL_CHAR(' ');
  SERIAL_CHAR('H');
  SERIAL_CHAR(':');
  SERIAL_CHAR('{');
  
  // plan position
  if (TEST_FLAG(selection, HeartbeatSelection::PLAN_POSITION))
  {
    SERIAL_CHAR('P');
    SERIAL_CHAR(':');
    SERIAL_CHAR('{');
    
    report_xyze(current_position.asLogical());
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
    xyze_long_t position = stepper.position();
    LOOP_XYZE(axis)
    {
        position[axis] /= planner.settings.axis_steps_per_mm[X_AXIS];
    }
  
    report_xyze(position.asLogical());
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
  
  // dummy data at end of json so that we don't have to worry about separators.
  SERIAL_CHAR('_');
  SERIAL_CHAR(':');
  SERIAL_CHAR('0');
  
  SERIAL_CHAR('}');
  // end heartbeat
}

void Heartbeat::auto_report() {
  if (heartbeat_interval && ELAPSED(millis(), next_heartbeat_report_ms)) {
    next_heartbeat_report_ms = millis() + heartbeat_interval;

    PORT_REDIRECT(SERIAL_BOTH);
    serial_info(Heartbeat::selection);
    SERIAL_EOL();
  }
}

} // namespace Rapidia

#endif // ENABLED(RAPIDIA_HEARTBEAT)