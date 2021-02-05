#pragma once

#include "../../inc/MarlinConfigPre.h"

#if ENABLED(RAPIDIA_HEARTBEAT)

namespace Rapidia
{

typedef uint8_t HeartbeatSelectionUint;

enum class HeartbeatSelection : HeartbeatSelectionUint
{
  PLAN_POSITION = _BV(0), // 'P'
  ABS_POSITION  = _BV(1), // 'C'
  RELMODE       = _BV(2), // 'R'
  FEEDRATE      = _BV(3), // 'F' (reported in P)
  DUALX         = _BV(4), // 'X'
  ENDSTOPS      = _BV(5), // 'E'
  DEBUG         = _BV(6), // 'D'
  ALL_POSITION = PLAN_POSITION | ABS_POSITION,
  _DEFAULT = PLAN_POSITION | ABS_POSITION | RELMODE | FEEDRATE | ENDSTOPS,
  _ALL = 0xff
};

class Heartbeat
{
public:
  static HeartbeatSelectionUint selection;
  
  // checks for heartbeat timer elapsed, if so, sends heartbeat message.
  static void auto_report();

  // sets heartbeat interval.
  static void set_interval(uint16_t ms);
  
  // sends status message
  // selection: what status to send
  // bare: if false, wrap message in H:{ on the left and } on the right"
  static void serial_info(HeartbeatSelection selection, bool bare=false);

  static inline void serial_info(HeartbeatSelectionUint selection, bool bare=false)
  {
    serial_info(static_cast<HeartbeatSelection>(selection));
  }

  #if ENABLED(RAPIDIA_PAUSE)
    // displays a message when block buffering/extrusion prevention ends after pause.
    static void pause_block_buffering_info();
  #endif
};

extern Heartbeat heartbeat;
}

#endif