#pragma once

#include "../../inc/MarlinConfigPre.h"
#include "../../core/millis_t.h"

#if ENABLED(RAPIDIA_MILEAGE)
namespace Rapidia
{

// accumulates printer data over time.
// (stored in EEPROM so this persists between runs, ideally.)

struct MileageData
{
    
    // all E steps taken.
    uint64_t e_steps;

    void update_crc();
    bool crc_check();
private:
    uint32_t crc32;
};

class Mileage
{
public:
    // this is updated by the stepper ISR,
    // and cleared in Mileageupdate()
    // 16 bits *ought* to be enough to store the maximum possible
    // number of steps taken between idle loops.
    static volatile uint16_t e_step_tally;

    // intended for stepper ISR use only.
    FORCE_INLINE static void increment_e_step_tally() { e_step_tally++; }

    static millis_t save_interval_ms;

    // updates mileage stats, possibly writes to eeprom.
    static void update();

    // retrieves data, loading from eeprom if necessary.
    static MileageData& Mileage::data();

private:
    // contains data for mileage. Can be written as a unit to EEPROM.
    static MileageData _data;
    static millis_t next_save_time_ms;

    // move e step tally into data.
    static void add_tally();

    static void load_eeprom();
    static void write_eeprom();
};

extern Mileage mileage;
}
#endif