#pragma once

#include "../../../Configuration.h"

#if ENABLED(RAPIDIA_STACK_UTIL)

#include "../../HAL/HAL.h"
#include <stdlib.h>

namespace Rapidia
{
    extern void* const avr_stack_end;
    extern void* const avr_stack_start;
    extern const uintptr_t avr_stack_size;
    extern void* stack_root;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wreturn-local-addr"
    inline void* get_stack_top()
    {
        volatile uint8_t v;
        return const_cast<uint8_t*>(&v);
    }

    inline uintptr_t get_stack_depth()
    {
        return reinterpret_cast<uintptr_t>(avr_stack_start)
            - reinterpret_cast<uintptr_t>(get_stack_top());
    }

    
    inline uintptr_t get_stack_remaining()
    {
        return reinterpret_cast<uintptr_t>(get_stack_top())
            - reinterpret_cast<uintptr_t>(avr_stack_end);
    }
    #pragma GCC diagnostic pop

    #ifdef RAPIDIA_STACK_USAGE
    uint16_t calc_stack_usage(void);
    #endif
}

#define RAPIDIA_MARK_STACK_ROOT() Rapidia::stack_root = __builtin_frame_address(0)

#else

#define RAPIDIA_MARK_STACK_ROOT()

#endif