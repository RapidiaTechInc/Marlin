#include "heartbeat.h"

#include "../../inc/MarlinConfig.h"
#include "../../module/stepper.h"
#include "../../module/planner.h"
#include "../../module/endstops.h"
#include "../../module/motion.h"
#include "../../gcode/gcode.h"

#if ENABLED(RAPIDIA_HEARTBEAT)

namespace Rapidia
{
static uint16_t heartbeat_interval_ms = 0;
static const uint16_t min_heartbeat_interval_ms = 80;
static millis_t next_heartbeat_report_ms = 0;
Heartbeat heartbeat; // singleton

HeartbeatSelectionUint Heartbeat::selection
  = (HeartbeatSelectionUint)HeartbeatSelection::_DEFAULT;

void Heartbeat::set_interval(uint16_t v)
{
  if (v && v < min_heartbeat_interval_ms)
  {
    v = min_heartbeat_interval_ms;
    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("Heartbeat interval is lower than minimum ");
    SERIAL_ECHO(min_heartbeat_interval_ms);
    SERIAL_ECHOLNPGM(" ms. Using minimum instead.");
  }

  heartbeat_interval_ms = v;
  next_heartbeat_report_ms = millis() + v;
}

static void report_homed()
{
  const bool x_homed = axis_homed & _BV(X_AXIS);
  const bool y_homed = axis_homed & _BV(Y_AXIS);
  const bool z_homed = axis_homed & _BV(Z_AXIS);
  if (x_homed) SERIAL_CHAR('x');
  if (y_homed) SERIAL_CHAR('y');
  if (z_homed) SERIAL_CHAR('z');
}

static void report_xyzetf(const xyze_pos_t &pos, const uint8_t extruder, const bool feedrate=false, const uint8_t n=XYZE, const uint8_t precision=3) {
  char str[12];

  // position.
  LOOP_L_N(a, n) {
    echo_key(axis_codes[a]);
    SERIAL_ECHO(dtostrf(pos[a], 1, precision, str));
    SERIAL_CHAR(',');
  }

  if (feedrate)
  {
    echo_key('F');
    SERIAL_ECHO(dtostrf(feedrate_mm_s, 1, precision, str));
    SERIAL_CHAR(',');
  }

  // extruder number
  echo_key('T');
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

  // separator accumulator
  bool sep = true;

  // plan position
  if (TEST_FLAG(selection, HeartbeatSelection::PLAN_POSITION))
  {
    echo_separator(sep);
    echo_key('P');
    SERIAL_CHAR('{');

    report_xyzetf(current_position.asLogical(), active_extruder, TEST_FLAG(selection, HeartbeatSelection::FEEDRATE));
    SERIAL_CHAR('}');

    echo_separator(sep);
    echo_key('H');
    SERIAL_CHAR('"');
    report_homed();
    SERIAL_CHAR('"');
  }

  // actual position
  if (TEST_FLAG(selection, HeartbeatSelection::ABS_POSITION))
  {
    echo_separator(sep);
    echo_key('C');
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
  }

  // relative mode axes:
  if (TEST_FLAG(selection, HeartbeatSelection::RELMODE))
  {
    echo_separator(sep);
    echo_key('R');
    SERIAL_CHAR('"');
    LOOP_XYZE(axis)
    {
      if (gcode.axis_is_relative(AxisEnum(axis)))
      {
        SERIAL_CHAR(axis_codes[axis]);
      }
    }
    SERIAL_CHAR('"');
  }

  // dualx info
  if (TEST_FLAG(selection, HeartbeatSelection::DUALX))
  {
    echo_separator(sep);
    echo_key('X');
    SERIAL_CHAR('{');
    {
      char str[12];

      // dual_x_carriage_mode
      echo_key('S');
      SERIAL_CHAR('0' + (int32_t)(dual_x_carriage_mode));
      SERIAL_CHAR(',');

      // active toolhead
      echo_key('T');
      SERIAL_CHAR('0' + (int32_t)(active_extruder));
      SERIAL_CHAR(',');

      // stored x position
      echo_key('X');
      SERIAL_ECHO(dtostrf(inactive_extruder_x_pos, 1, 3, str));

      // TODO: stored feedrate.
    }
    SERIAL_CHAR('}');
  }

  // endstops -- report endstops closed state (at this moment)
  if (TEST_FLAG(selection, HeartbeatSelection::ENDSTOPS))
  {
    echo_separator(sep);
    echo_key('E');
    SERIAL_CHAR('"');

    // read from the endstop pins directly.
    // (this info doesn't seem to be cached in the Endstops class.)

    #define ES_REPORT(S, N) if (endstops.endstop_state(S)) SERIAL_CHAR(N);

    ES_REPORT(X_MIN, 'x');
    ES_REPORT(Y_MIN, 'y');
    ES_REPORT(Z_MIN, 'z');
    ES_REPORT(X_MAX, 'X');
    ES_REPORT(Y_MAX, 'Y');
    ES_REPORT(Z_MAX, 'Z');

    SERIAL_CHAR('"');

    if (TEST_FLAG(selection, HeartbeatSelection::DEBUG))
    {
      echo_separator(sep);
      echo_key_str("dbg-executing-command");
      if (GcodeSuite::dbg_current_command_letter)
      {
        char sbuff[32];
        if (sprintf(sbuff, "\"%c%d\"", GcodeSuite::dbg_current_command_letter, GcodeSuite::dbg_current_codenum) > 0)
        {
          SERIAL_ECHO(sbuff);
        }
        else
        {
          SERIAL_ECHO("\"\"");
        }
      }
      else
      {
        SERIAL_ECHO("\"\"");
      }
      

      echo_separator(sep);
      echo_key_str("dbg-pause-nobuffer");
      SERIAL_ECHO(static_cast<int32_t>(planner.prevent_block_buffering));
      
      echo_separator(sep);
      echo_key_str("dbg-pause-noextrude");
      SERIAL_ECHO(static_cast<int32_t>(planner.prevent_block_extrusion));

      echo_separator(sep);
      echo_key_str("dbg-buffer-moves-planned");
      SERIAL_ECHO(static_cast<int32_t>(planner.movesplanned()));

      echo_separator(sep);
      echo_key_str("dbg-buffer-moves-nonbusy");
      SERIAL_ECHO(static_cast<int32_t>(planner.nonbusy_movesplanned()));

      echo_separator(sep);
      echo_key_str("dbg-live-state");
      SERIAL_ECHO(static_cast<int32_t>(endstops.live_state));

      echo_separator(sep);
      echo_key_str("dbg-state()");
      SERIAL_ECHO(static_cast<int32_t>(endstops.state()));

      echo_separator(sep);
      echo_key_str("dbg-enable");
      SERIAL_ECHO(endstops.enabled);

      echo_separator(sep);
      echo_key_str("dbg-enable-globally");
      SERIAL_ECHO(endstops.enabled_globally);

      echo_separator(sep);
      echo_key_str("dbg-hit-state");
      SERIAL_ECHO(static_cast<int32_t>(endstops.hit_state));

      #if ENABLED(RAPIDIA_NOZZLE_PLUG_HYSTERESIS)
      echo_separator(sep);
      echo_key_str("dbg-zmax-hyst-count");
      SERIAL_ECHO(static_cast<int32_t>(endstops.z_max_hysteresis_count));

      echo_separator(sep);
      echo_key_str("dbg-zmax-hyst-threshold");
      SERIAL_ECHO(static_cast<int32_t>(endstops.z_max_hysteresis_threshold));
      #endif
    }
  }

  if (!bare)
  {
    SERIAL_ECHOPGM("}");
  }
  // end heartbeat
}

void Heartbeat::auto_report()
{
  if (heartbeat_interval_ms && ELAPSED(millis(), next_heartbeat_report_ms)) {
    next_heartbeat_report_ms = millis() + heartbeat_interval_ms;

    PORT_REDIRECT(SERIAL_BOTH);
    serial_info(static_cast<HeartbeatSelection>(Heartbeat::selection));
    SERIAL_EOL();
  }
}

#if ENABLED(RAPIDIA_PAUSE)
void Heartbeat::pause_block_buffering_info()
{
  HeartbeatSelection sel = static_cast<HeartbeatSelection>(Heartbeat::selection);
  if (TEST_FLAG(sel, HeartbeatSelection::DEBUG))
  {
    // if either of these flags are true, then the caller is about
    // to disable them. So we report that they have been disabled.
    if (planner.prevent_block_buffering)
    {
      SERIAL_ECHO_START();
      SERIAL_ECHOLN("pause: dbg-pause-nobuffer disabled.");
    }
    if (planner.prevent_block_extrusion)
    {
      SERIAL_ECHO_START();
      SERIAL_ECHOLN("pause: dbg-pause-noextrude disabled.");
    }
  }
}
#endif

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
