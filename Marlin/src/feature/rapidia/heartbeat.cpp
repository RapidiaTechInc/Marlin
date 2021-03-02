#include "heartbeat.h"

#include "checksum.h"
#include "mileage.h"

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
  = static_cast<HeartbeatSelectionUint>(HeartbeatSelection::_DEFAULT);

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

static void report_homed(CHK_ARGDEF)
{
  const bool x_homed = axis_homed & _BV(X_AXIS);
  const bool y_homed = axis_homed & _BV(Y_AXIS);
  const bool z_homed = axis_homed & _BV(Z_AXIS);
  if (x_homed) SERIAL_CHAR_CHK('x');
  if (y_homed) SERIAL_CHAR_CHK('y');
  if (z_homed) SERIAL_CHAR_CHK('z');
  if (homing_semaphore) SERIAL_CHAR_CHK('h');
}

static void report_xyzetf(CHK_ARGSDEF const xyze_pos_t &pos, const uint8_t extruder, const bool feedrate=false, const uint8_t n=XYZE, const uint8_t precision=3) {
  char str[12];

  // position.
  LOOP_L_N(a, n) {
    ECHO_KEY_CHK(axis_codes[a]);
    SERIAL_ECHO_CHK(dtostrf(pos[a], 1, precision, str));
    SERIAL_CHAR_CHK(',');
  }

  if (feedrate)
  {
    ECHO_KEY_CHK('F');
    SERIAL_ECHO_CHK(dtostrf(feedrate_mm_s, 1, precision, str));
    SERIAL_CHAR_CHK(',');
  }

  // extruder number
  ECHO_KEY_CHK('T');
  SERIAL_CHAR_CHK('0' + extruder);
}

#define TEST_FLAG(a, b) (!!((uint32_t)(a) & (uint32_t)(b)))

void Heartbeat::serial_info(HeartbeatSelection selection, bool bare)
{
  #if ENABLED(RAPIDIA_MILEAGE)
    const MileageData* mileage_data;
    if (TEST_FLAG(selection, HeartbeatSelection::MILEAGE))
    {
      mileage_data = &mileage.data();
    }
  #endif

  static char chbuff[32];

  SERIAL_INIT_CHECKSUM();

  // begin heartbeat
  if (!bare)
  {
    SERIAL_ECHO_CHK("H:{");
  }

  // separator accumulator
  bool sep = true;

  // plan position
  if (TEST_FLAG(selection, HeartbeatSelection::PLAN_POSITION))
  {
    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_CHK('P');
    SERIAL_CHAR_CHK('{');

    report_xyzetf(CHK_ARGS current_position.asLogical(), active_extruder, TEST_FLAG(selection, HeartbeatSelection::FEEDRATE));
    SERIAL_CHAR_CHK('}');

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_CHK('H');
    SERIAL_CHAR_CHK('"');
    report_homed(CHK_ARG);
    SERIAL_CHAR_CHK('"');
  }

  // actual position
  if (TEST_FLAG(selection, HeartbeatSelection::ABS_POSITION))
  {
    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_CHK('C');
    SERIAL_CHAR_CHK('{');

    // unit conversion steps -> logical
    Stepper::State state = stepper.report_state();
    xyze_pos_t position;
    LOOP_XYZE(axis)
    {
      position[axis] = state.position[axis] / planner.settings.axis_steps_per_mm[axis];
    }

    report_xyzetf(CHK_ARGS position.asLogical(), state.extruder);
    SERIAL_CHAR_CHK('}');
  }

  // relative mode axes:
  if (TEST_FLAG(selection, HeartbeatSelection::RELMODE))
  {
    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_CHK('R');
    SERIAL_CHAR_CHK('"');
    LOOP_XYZE(axis)
    {
      if (gcode.axis_is_relative(AxisEnum(axis)))
      {
        SERIAL_CHAR_CHK(axis_codes[axis]);
      }
    }
    SERIAL_CHAR_CHK('"');
  }

  // dualx info
  if (TEST_FLAG(selection, HeartbeatSelection::DUALX))
  {
    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_CHK('X');
    SERIAL_CHAR_CHK('{');
    {
      char str[12];

      // dual_x_carriage_mode
      ECHO_KEY_CHK('S');
      SERIAL_CHAR_CHK('0' + (int32_t)(dual_x_carriage_mode));
      SERIAL_CHAR_CHK(',');

      // active toolhead
      ECHO_KEY_CHK('T');
      SERIAL_CHAR_CHK('0' + (int32_t)(active_extruder));
      SERIAL_CHAR_CHK(',');

      // stored x position
      ECHO_KEY_CHK('X');
      SERIAL_ECHO_CHK(dtostrf(inactive_extruder_x_pos, 1, 3, str));

      // TODO: stored feedrate.
    }
    SERIAL_CHAR_CHK('}');
  }

  if (TEST_FLAG(selection, HeartbeatSelection::MILEAGE))
    {
      ECHO_SEPARATOR_CHK(sep);
      ECHO_KEY_CHK('M');
      #if ENABLED(RAPIDIA_MILEAGE)
      SERIAL_CHAR_CHK('{');
      for (uint8_t e = 0; e < EXTRUDERS; ++e)
      {
        SERIAL_CHAR_CHK('"');
        SERIAL_CHAR_CHK('E');
        SERIAL_CHAR_CHK(('1' + e));
        static_assert(EXTRUDERS < 9, "at most 9 extruders function for this arithmetic.");
        SERIAL_CHAR_CHK('"');
        SERIAL_CHAR_CHK(':');

        uint64_t val = mileage_to_u64nm(mileage_data->e_mm[e]);
        // convert to mm/1000
        constexpr uint8_t PREC = 3;
        val /= 1000; // this must be 10^(6 - PREC), where 6 is the digits between nm and mm.

        // make this into floating point by shifting digits over and inserting a '.'
        const char* c = (val >= 1000) // 1000 = 10^PREC
            ? (_sprint_dec(chbuff + 1, val, sizeof(chbuff) - 1, false) - 1)
            : (_sprint_dec(chbuff + sizeof(chbuff) - PREC - 1, val, PREC + 1, true) - 1);
        for (size_t i = 0; i < sizeof(chbuff) - 1 - PREC; ++i)
        {
          chbuff[i] = chbuff[i + 1];
        }
        chbuff[sizeof(chbuff) - 1 - PREC] = '.';
        SERIAL_ECHO_CHK(c);
        SERIAL_CHAR_CHK(',');
      }
      ECHO_KEY_CHK('I');
      SERIAL_ECHO_CHK(itoa(mileage.get_save_index(), chbuff, 10));
      if (mileage.get_expended())
      {
        SERIAL_CHAR_CHK(',');
        SERIAL_ECHOPGM_CHK("\"expended\":true");
      }
      SERIAL_CHAR_CHK('}');
      #else
      SERIAL_ECHO_CHK("null");
      #endif
    }

  // endstops -- report endstops closed state (at this moment)
  if (TEST_FLAG(selection, HeartbeatSelection::ENDSTOPS))
  {
    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_CHK('E');
    SERIAL_CHAR_CHK('"');

    // read from the endstop pins directly.
    // (this info doesn't seem to be cached in the Endstops class.)

    #define ES_REPORT(S, N) if (endstops.endstop_state(S)) SERIAL_CHAR_CHK(N);

    ES_REPORT(X_MIN, 'x');
    ES_REPORT(Y_MIN, 'y');
    ES_REPORT(Z_MIN, 'z');
    ES_REPORT(X_MAX, 'X');
    ES_REPORT(Y_MAX, 'Y');
    ES_REPORT(Z_MAX, 'Z');

    SERIAL_CHAR_CHK('"');
  }

  if (TEST_FLAG(selection, HeartbeatSelection::DEBUG))
  {
    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-executing-command");
    if (GcodeSuite::dbg_current_command_letter)
    {
      if (sprintf(chbuff, "\"%c%d\"", GcodeSuite::dbg_current_command_letter, GcodeSuite::dbg_current_codenum) > 0)
      {
        SERIAL_ECHO_CHK(chbuff);
      }
      else
      {
        SERIAL_ECHO_CHK("\"\"");
      }
    }
    else
    {
      SERIAL_ECHO_CHK("\"\"");
    }


    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-pause-nobuffer");
    SERIAL_ECHO_CHK(itoa(planner.prevent_block_buffering, chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-pause-noextrude");
    SERIAL_ECHO_CHK(itoa(planner.prevent_block_extrusion, chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-buffer-moves-planned");
    SERIAL_ECHO_CHK(itoa(planner.movesplanned(), chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-buffer-moves-nonbusy");
    SERIAL_ECHO_CHK(itoa(planner.nonbusy_movesplanned(), chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-live-state");
    SERIAL_ECHO_CHK(itoa(endstops.live_state, chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-state()");
    SERIAL_ECHO_CHK(itoa(endstops.state(), chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-enable");
    SERIAL_ECHO_CHK(itoa(endstops.enabled, chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-enable-globally");
    SERIAL_ECHO_CHK(itoa(endstops.enabled_globally, chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-hit-state");
    SERIAL_ECHO_CHK(itoa(endstops.hit_state, chbuff, 10));

    #if ENABLED(RAPIDIA_NOZZLE_PLUG_HYSTERESIS)
    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-zmax-hyst-count");
    SERIAL_ECHO_CHK(itoa(endstops.z_max_hysteresis_count, chbuff, 10));

    ECHO_SEPARATOR_CHK(sep);
    ECHO_KEY_STR_CHK("dbg-zmax-hyst-threshold");
    SERIAL_ECHO_CHK(itoa(endstops.z_max_hysteresis_threshold, chbuff, 10));
    #endif
  }

  if (!bare)
  {
    SERIAL_ECHOLNPGM_CHK("}");
  }
  // end heartbeat
}

void Heartbeat::auto_report()
{
  if (heartbeat_interval_ms && ELAPSED(millis(), next_heartbeat_report_ms)) {
    next_heartbeat_report_ms = millis() + heartbeat_interval_ms;

    PORT_REDIRECT(SERIAL_BOTH);
    serial_info(Heartbeat::selection);
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

} // namespace Rapidia

#endif // ENABLED(RAPIDIA_HEARTBEAT)
