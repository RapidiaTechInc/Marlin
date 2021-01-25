#include "kill_recovery.h"
#include "../../core/serial.h"

#if ENABLED(RAPIDIA_KILL_RECOVERY)

#define VEC(A) A##_vect_num

namespace Rapidia
{

// this is the 0-terminated list of interrupts that
// must be disabled on minkill.
// Please ensure that there is an associated handler
// in the disable_interrupt() function.
const volatile uint8_t minkill_interrupts_off[] PROGMEM
{
    VEC(TIMER0_COMPB),
    VEC(TIMER1_COMPA),
    0
};

// interrupts that may remain enabled after minkill.
const volatile uint8_t minkill_interrupts_on[] PROGMEM 
{
    VEC(TIMER0_OVF),  // millis()
    VEC(USART0_RX),   // serial input
    VEC(USART0_UDRE), // serial output
    0
};

// Note: the above arrays must cover the
// full set of interrupts used in the firmware.

// disables all interrupts in the given zero-terminated list.
void disable_interrupts_pgm(const volatile uint8_t* interrupts)
{
    uint8_t interrupt;
    while (interrupt = pgm_read_byte(interrupts))
    {
        disable_interrupt(interrupt);
        interrupts++;
    }
}

void disable_interrupt(uint8_t interrupt)
{
    switch (interrupt)
    {
        case 0:
            // ignore zero (would be the reset vector)
            return;
        case VEC(INT0):
            CBI(EIMSK, INT0);
            CBI(EIFR, INTF0);
            break;
        case VEC(INT1):
            CBI(EIMSK, INT1);
            CBI(EIFR, INTF1);
            break;
        case VEC(INT2):
            CBI(EIMSK, INT2);
            CBI(EIFR, INTF2);
            break;
        case VEC(INT3):
            CBI(EIMSK, INT3);
            CBI(EIFR, INTF3);
            break;
        case VEC(INT4):
            CBI(EIMSK, INT4);
            CBI(EIFR, INTF4);
            break;
        case VEC(INT5):
            CBI(EIMSK, INT5);
            CBI(EIFR, INTF5);
            break;
        case VEC(INT6):
            CBI(EIMSK, INT6);
            CBI(EIFR, INTF6);
            break;
        case VEC(INT7):
            CBI(EIMSK, INT7);
            CBI(EIFR, INTF7);
            break;
        case VEC(TIMER0_OVF):
            CBI(TIMSK0, TOIE0);
            CBI(TIFR0, TOV0);
            break;
        case VEC(TIMER0_COMPA):
            CBI(TIMSK0, OCIE0A);
            CBI(TIFR0, OCF0A);
            break;
        case VEC(TIMER0_COMPB):
            CBI(TIMSK0, OCIE0B);
            CBI(TIFR0, OCF0B);
            break;
        case VEC(TIMER1_OVF):
            CBI(TIMSK1, TOIE1);
            CBI(TIFR1, TOV1);
            break;
        case VEC(TIMER1_CAPT):
            CBI(TIMSK1, ICIE1);
            CBI(TIFR1, ICF1);
            break;
        case VEC(TIMER1_COMPA):
            CBI(TIMSK1, OCIE1A);
            CBI(TIFR1, OCF1A);
            break;
        case VEC(TIMER1_COMPB):
            CBI(TIMSK1, OCIE1B);
            CBI(TIFR1, OCF1B);
            break;
        case VEC(TIMER1_COMPC):
            CBI(TIMSK1, OCIE1C);
            CBI(TIFR1, OCF1C);
            break;
        case VEC(TIMER2_OVF):
            CBI(TIMSK2, TOIE2);
            CBI(TIFR2, TOV2);
            break;
        case VEC(TIMER2_COMPA):
            CBI(TIMSK2, OCIE2A);
            CBI(TIFR2, OCF2A);
            break;
        case VEC(TIMER2_COMPB):
            CBI(TIMSK2, OCIE2B);
            CBI(TIFR2, OCF2B);
            break;
        case VEC(TIMER3_OVF):
            CBI(TIMSK3, TOIE3);
            CBI(TIFR3, TOV3);
            break;
        case VEC(TIMER3_CAPT):
            CBI(TIMSK3, ICIE3);
            CBI(TIFR3, ICF3);
            break;
        case VEC(TIMER3_COMPA):
            CBI(TIMSK3, OCIE3A);
            CBI(TIFR3, OCF3A);
            break;
        case VEC(TIMER3_COMPB):
            CBI(TIMSK3, OCIE3B);
            CBI(TIFR3, OCF3B);
            break;
        case VEC(TIMER3_COMPC):
            CBI(TIMSK3, OCIE3C);
            CBI(TIFR3, OCF3C);
            break;
        case VEC(TIMER4_OVF):
            CBI(TIMSK4, TOIE4);
            CBI(TIFR4, TOV4);
            break;
        case VEC(TIMER4_CAPT):
            CBI(TIMSK4, ICIE4);
            CBI(TIFR4, ICF4);
            break;
        case VEC(TIMER4_COMPA):
            CBI(TIMSK4, OCIE4A);
            CBI(TIFR4, OCF4A);
            break;
        case VEC(TIMER4_COMPB):
            CBI(TIMSK4, OCIE4B);
            CBI(TIFR4, OCF4B);
            break;
        case VEC(TIMER4_COMPC):
            CBI(TIMSK4, OCIE4C);
            CBI(TIFR4, OCF4C);
            break;
        case VEC(TIMER5_OVF):
            CBI(TIMSK5, TOIE5);
            CBI(TIFR5, TOV5);
            break;
        case VEC(TIMER5_CAPT):
            CBI(TIMSK5, ICIE5);
            CBI(TIFR5, ICF5);
            break;
        case VEC(TIMER5_COMPA):
            CBI(TIMSK5, OCIE5A);
            CBI(TIFR5, OCF5A);
            break;
        case VEC(TIMER5_COMPB):
            CBI(TIMSK5, OCIE5B);
            CBI(TIFR5, OCF5B);
            break;
        case VEC(TIMER5_COMPC):
            CBI(TIMSK5, OCIE5C);
            CBI(TIFR5, OCF5C);
            break;
        // TODO: handle all other interrupts.
        default:
            // error message.
            SERIAL_ERROR_START();
            SERIAL_ECHOPGM("No disable routine for interrupt ");
            SERIAL_ECHOLN(interrupt);
            return;
    }
}

}
#endif