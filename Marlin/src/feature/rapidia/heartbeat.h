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
  _DEFAULT = PLAN_POSITION | ABS_POSITION | RELMODE | FEEDRATE
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
  static void serial_info(HeartbeatSelection selection);
  
  // enables/disables individual status messages
  static void select(HeartbeatSelection selection, bool enable);
};

extern Heartbeat heartbeat;
}

#endif