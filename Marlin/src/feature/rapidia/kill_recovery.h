#pragma once

#include "../../inc/MarlinConfigPre.h"
#include <avr/pgmspace.h>

#if ENABLED(RAPIDIA_KILL_RECOVERY)

namespace Rapidia
{
  extern const volatile uint8_t minkill_interrupts_off[] PROGMEM;
  extern const volatile uint8_t minkill_interrupts_on[] PROGMEM;

  // disables the given zero-terminated list of interrupts.
  // (array must be in progmem)
  void disable_interrupts_pgm(const volatile uint8_t*);

  // disables the given interrupt.
  void disable_interrupt(uint8_t);
}

#endif