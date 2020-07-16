#include "MarlinConfig.h"

// these hooks allow the emulator to read certain read-only data.

#if ENABLED(RAPIDIA_EMULATOR_HOOKS)

// hook a value defined in flash memory
#define FLASH(type, name, value)              \
namespace hooks { namespace k_##type { \
const volatile type f_##name PROGMEM = value; \
} }

// hook a macro defined in flash memory.
#define MACRO(type, name, prefix_) FLASH(type, prefix_##name, prefix_##name)

namespace Rapidia
{
    MACRO(float, X_MIN_POS,)
    MACRO(float, X_MAX_POS,)
    MACRO(float, Y_MIN_POS,)
    MACRO(float, Y_MAX_POS,)
    MACRO(float, Z_MIN_POS,)
    MACRO(float, Z_MAX_POS,)
    MACRO(float, X_BED_SIZE,)
    MACRO(float, Y_BED_SIZE,)

    // emulator's starting positions for toolheads.
    FLASH(float, X_START_POS, 0)
    FLASH(float, X2_START_POS, X_BED_SIZE)
    FLASH(float, Y_START_POS, Y_BED_SIZE / 2)
    FLASH(float, Z_START_POS, 1)

    // steps per unit for toolheads
    static constexpr float spu[] = DEFAULT_AXIS_STEPS_PER_UNIT;
    FLASH(float, X_STEPS_PER_UNIT, spu[0])
    FLASH(float, Y_STEPS_PER_UNIT, spu[1])
    FLASH(float, Z_STEPS_PER_UNIT, spu[2])
    FLASH(float, E_STEPS_PER_UNIT, spu[3])

} // namespace Rapidia

#endif
