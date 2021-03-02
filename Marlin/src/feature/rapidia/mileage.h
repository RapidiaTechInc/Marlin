#pragma once

#include "../../inc/MarlinConfigPre.h"
#include "../../core/millis_t.h"
#include <stdint.h>

#if ENABLED(RAPIDIA_MILEAGE)
namespace Rapidia
{

typedef uint64_t mileage_amount_t;

// this is the precision at which mileage is stored -- in nanometres
constexpr uint64_t MILEAGE_FIXED_PRECISION = 1000000;

inline double mileage_to_double(mileage_amount_t a)
{
  uint64_t integral = (a / MILEAGE_FIXED_PRECISION);
  uint64_t floating = static_cast<double>(a % MILEAGE_FIXED_PRECISION);

  return static_cast<double>(integral) + static_cast<double>(floating) / static_cast<double>(MILEAGE_FIXED_PRECISION);
}

inline mileage_amount_t double_to_mileage(double v)
{
  return v * MILEAGE_FIXED_PRECISION;
}

inline mileage_amount_t u64nm_to_mileage(uint64_t v)
{
  static_assert(1000000 == MILEAGE_FIXED_PRECISION,
    "this function relies on mileage being stored in nm."
  );

  return v;
}

inline uint64_t mileage_to_u64nm(mileage_amount_t a)
{
  static_assert(1000000 == MILEAGE_FIXED_PRECISION,
    "this function relies on mileage being stored in nm."
  );

  return a;
}

// accumulates printer data over time.
// (stored in EEPROM so this persists between runs, ideally.)

struct MileageData
{
    // All E "length" of theoretical filament extruded in mm
    // Preserved raw directly from gcode command
    mileage_amount_t e_mm[EXTRUDERS];

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
    static void increment_e_mm_tally(uint8_t extruder, mileage_amount_t e_mm) {
      e_mm_tally[extruder] += e_mm;
    }

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

    // This is updated in the planner, less accurate that actual stepper isr, but good enough for
    // the purpose of tracking paste usage.
    static mileage_amount_t e_mm_tally[EXTRUDERS];
};

extern Mileage mileage;
}
#endif
