#include "stack_util.h"
#include "../../MarlinCore.h"
#include "../../core/serial.h"

#if ENABLED(RAPIDIA_STACK_UTIL)

// these symbols are set by the AVR linker, referring to
// the stack start and end.
extern uint8_t _end;
extern uint8_t __stack;

namespace Rapidia
{
    void* stack_root;
    void* const avr_stack_end { &_end };
    void* const avr_stack_start { &__stack };
    const uintptr_t avr_stack_size {
        reinterpret_cast<uintptr_t>(avr_stack_start) - reinterpret_cast<uintptr_t>(avr_stack_end)
    };

    #ifdef RAPIDIA_STACK_USAGE
    uint16_t calc_stack_usage(void)
    {
        assert_kill_pgm(avr_stack_end < avr_stack_start, PSTR("stack direction unexpected."));

        const uint8_t *p = &_end;
        uint16_t       c = avr_stack_size;

        #if __GNUC__ >= 8
            #pragma GCC unroll 16
            while(p <= &__stack)
        #else
            while(p <= &__stack)
        #endif
        {
            if (*p != 0xc5) break;
            p++;
            c--;
        }

        return c;
    }
    #endif // RAPIDIA_STACK_USAGE
}

#ifdef RAPIDIA_STACK_USAGE
// https://www.avrfreaks.net/forum/soft-c-avrgcc-monitoring-stack-usage
// (public domain)

void StackPaint(void) __attribute__ ((naked)) __attribute__ ((section (".init1")));

void StackPaint(void)
{
#if 0
    uint8_t *p = &_end;

    while(p <= &__stack)
    {
        *p = STACK_CANARY;
        p++;
    }
#else
    __asm volatile ("    ldi r30,lo8(_end)\n"
                    "    ldi r31,hi8(_end)\n"
                    "    ldi r24,lo8(0xc5)\n" /* STACK_CANARY = 0xc5 */
                    "    ldi r25,hi8(__stack)\n"
                    "    rjmp .cmp\n"
                    ".loop:\n"
                    "    st Z+,r24\n"
                    ".cmp:\n"
                    "    cpi r30,lo8(__stack)\n"
                    "    cpc r31,r25\n"
                    "    brlo .loop\n"
                    "    breq .loop"::);
#endif
}
#endif
#endif