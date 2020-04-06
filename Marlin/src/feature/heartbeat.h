#pragma once

#include "../inc/MarlinConfigPre.h"

#if ENABLED(RAPIDIA_HEARTBEAT)

// checks for heartbeat timer elapsed, if so, sends heartbeat message.
void rapidia_heartbeat();

// sets heartbeat interval.
void rapidia_heartbeat_set_interval(uint16_t ms);

#endif