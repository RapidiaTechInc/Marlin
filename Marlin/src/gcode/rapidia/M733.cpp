#include "../../inc/MarlinConfig.h"
#include "../gcode.h"

#if ENABLED(RAPIDIA_PIN_TEST)

namespace
{
    typedef void (*pin_pulse_t)(bool v);

    struct pin_test_t {
        pin_pulse_t m_pulse;
    };
}

#define PIN_DEF(pin)    { \
    [](bool v) { \
        WRITE(pin, v); \
    } \
},

void GcodeSuite::M733()
{
    const static pin_test_t pins[] = {
        PIN_DEF(X_STEP_PIN)
        PIN_DEF(X2_STEP_PIN)
        PIN_DEF(Y_STEP_PIN)
        PIN_DEF(Z_STEP_PIN)
        PIN_DEF(E0_STEP_PIN)
        PIN_DEF(E1_STEP_PIN)
        PIN_DEF(X_DIR_PIN)
        PIN_DEF(X2_DIR_PIN)
        PIN_DEF(Y_DIR_PIN)
        PIN_DEF(Z_DIR_PIN)
        PIN_DEF(E0_DIR_PIN)
        PIN_DEF(E1_DIR_PIN)
        PIN_DEF(X_ENABLE_PIN)
        PIN_DEF(X2_ENABLE_PIN)
        PIN_DEF(Y_ENABLE_PIN)
        PIN_DEF(Z_ENABLE_PIN)
        PIN_DEF(E0_ENABLE_PIN)
        PIN_DEF(E1_ENABLE_PIN)
    };

    // prevent interrupts
    cli();

    for (uint8_t j = 0; j < 2; ++j)
    {
        for (size_t i = 0; i < sizeof(pins) / sizeof(pin_test_t); ++i)
        {
            pins[i].m_pulse(!j);
        }
    }

    // allow interrupts
    sei();
}
#endif // RAPIDIA_PIN_TEST

