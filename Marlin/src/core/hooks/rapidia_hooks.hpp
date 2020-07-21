#include "../../inc/MarlinConfig.h"

// these hooks allow the emulator to read certain read-only data.

#if ENABLED(RAPIDIA_EMULATOR_HOOKS)

// hook a value defined in flash memory
#define FLASH(type, name, value)              \
namespace hooks { namespace k_##type { \
const volatile type f_##name PROGMEM = value; \
} }

// hook a value defined in data memory
#define DATA(type, name, value)              \
namespace hooks { namespace k_##type { \
volatile type s_##name = value; \
} }


// hook a macro defined in flash memory.
#define MACRO(type, name, prefix_) FLASH(type, prefix_##name, prefix_##name)

#endif