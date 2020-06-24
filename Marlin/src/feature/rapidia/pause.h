#pragma once

#include "../../inc/MarlinConfig.h"

#if ENABLED(RAPIDIA_PAUSE)
namespace Rapidia
{
class Pause
{
  //static uint8_t defer_pause; // 0: no pause deferred. 1: soft. 2: hard.
  public:
    static void pause(bool hard);
    static void defer(bool hard); // pauses next idle update
    static void process_deferred();
};
  
extern Pause pause;
}
#endif