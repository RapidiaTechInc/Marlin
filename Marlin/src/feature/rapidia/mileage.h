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
    // all E steps taken (on both extruders).
    uint64_t e_steps[EXTRUDERS];

    void update_crc();
    bool crc_check();

private:
    // this must be the last member.
    uint16_t crc;
    uint16_t calc_crc();
};

class Mileage
{
public:
    // intended for stepper ISR use only.
    FORCE_INLINE static void increment_e_step_tally(uint8_t e) { e_step_tally[e]++; }

    static millis_t save_interval_ms;

    // updates mileage stats, possibly saves to eeprom.
    static void update();

    // retrieves data, loading from eeprom if necessary.
    static MileageData& data();

    // force immediate load or save
    static bool load_eeprom();
    static bool save_eeprom();

    // resets mileage (and possibly saves this to eeprom)
    static void reset(bool save=false);

    static uint8_t get_save_index();
    static bool get_expended();
    static bool get_loaded();

private:
    // contains data for mileage. Can be written as a unit to EEPROM.
    static MileageData _data;
    static millis_t next_save_time_ms;

    // reasons why loading/saving can fail.
    enum class ErrorCode {
        CRC_MISMATCH,
        FORMAT,
        EXPENDED,
        HEADER_DAMAGE,
        FIRST_TIME
    };

    // move e step tally into data.
    static void add_tally();

    static bool read_header();
    static bool write_header();
    static bool header_is_empty();
    static bool load_fail(ErrorCode);
    static bool save_fail(ErrorCode);
    static void fail(ErrorCode); // helper for load_fail and save_fail

    // this is updated by the stepper ISR,
    // and cleared in Mileageupdate()
    // 24 bits ought to be enough to store the maximum possible
    // number of steps taken between idle loops.
    // maximum
    static __uint24 e_step_tally[EXTRUDERS];

    static_assert(sizeof(e_step_tally) == EXTRUDERS * 3, "e step tally not compressed.");
};

extern Mileage mileage;
}
#endif