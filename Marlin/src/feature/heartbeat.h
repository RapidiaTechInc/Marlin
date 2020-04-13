#pragma once

#include "../inc/MarlinConfigPre.h"

#if ENABLED(RAPIDIA_HEARTBEAT)

namespace Rapidia
{

enum class HeartbeatSelection : uint8_t
{
  PLAN_POSITION = _BV(0),
  ABS_POSITION = _BV(1),
  RELMODE = _BV(2),
  ALL = PLAN_POSITION | ABS_POSITION | RELMODE
};

class Heartbeat
{
public:
  static uint8_t selection;
  
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