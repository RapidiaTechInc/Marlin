#include "heartbeat.h"

#include "../inc/MarlinConfig.h"
#include "../module/stepper.h"
#include "../module/planner.h"

#if ENABLED(RAPIDIA_HEARTBEAT)

uint16_t heartbeat_interval = 0;
millis_t next_heartbeat_report_ms = 0;

void rapidia_heartbeat_set_interval(uint16_t v)
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

static void print_heartbeat()
{
  // begin heartbeat
  SERIAL_CHAR(' ');
  SERIAL_CHAR('H');
  SERIAL_CHAR(':');
  SERIAL_CHAR('{');
  
  // begin plan position
  SERIAL_CHAR('P');
  SERIAL_CHAR(':');
  SERIAL_CHAR('{');
  
  report_xyze(current_position.asLogical());
  SERIAL_CHAR('}');
  SERIAL_CHAR(',');
  // end plan position
  
  // begin actual position
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
  // end actual position
  
  SERIAL_CHAR('}');
  // end heartbeat
}

void rapidia_heartbeat() {
  if (heartbeat_interval && ELAPSED(millis(), next_heartbeat_report_ms)) {
    next_heartbeat_report_ms = millis() + heartbeat_interval;

    PORT_REDIRECT(SERIAL_BOTH);
    print_heartbeat();
    SERIAL_EOL();
  }
}

#endif // ENABLED(RAPIDIA_HEARTBEAT)