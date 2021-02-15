#include "rapidia_hooks.hpp"
#include "../../HAL/HAL.h"

#if ENABLED(RAPIDIA_EMULATOR_HOOKS)

namespace Rapidia
{
    MACRO(uint32_t, BAUDRATE,)
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
    FLASH(float, Y_START_POS, Y_BED_SIZE * 5.0 / 6.0)
    FLASH(float, Z_START_POS, 1)

    // steps per unit for toolheads
    static constexpr float spu[] = DEFAULT_AXIS_STEPS_PER_UNIT;
    FLASH(float, X_STEPS_PER_UNIT, spu[0])
    FLASH(float, Y_STEPS_PER_UNIT, spu[1])
    FLASH(float, Z_STEPS_PER_UNIT, spu[2])
    FLASH(float, E_STEPS_PER_UNIT, spu[3])

    // endstop inverting (1 means inverted)
    FLASH(
        uint8_t, ENDSTOP_INVERTING,
        (X_MIN_ENDSTOP_INVERTING << 0) |
        (Y_MIN_ENDSTOP_INVERTING << 1) |
        (Z_MIN_ENDSTOP_INVERTING << 2) |
        (X_MAX_ENDSTOP_INVERTING << 3) |
        (Y_MAX_ENDSTOP_INVERTING << 4) |
        (Z_MAX_ENDSTOP_INVERTING << 5)
    )

    // enable inverting (0 means inverted)
    FLASH(
        uint8_t, ENABLE_ON,
        (X_ENABLE_ON << 0) |
        (Y_ENABLE_ON << 1) |
        (Z_ENABLE_ON << 2) |
        (E_ENABLE_ON << 3) |
        (E_ENABLE_ON << 4)
    )

    // direction inverting (1 means inverted)
    FLASH(
        uint8_t, INVERT_DIR,
        (INVERT_X_DIR << 0) |
        (INVERT_Y_DIR << 1) |
        (INVERT_Z_DIR << 2) |
        (INVERT_E0_DIR << 3) |
        (INVERT_E1_DIR << 4)
    )
} // namespace Rapidia

#endif