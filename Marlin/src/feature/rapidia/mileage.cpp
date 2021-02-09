#include "mileage.h"

#include "../../core/serial.h"

#include <avr/interrupt.h>

#if ENABLED(RAPIDIA_MILEAGE)

#define RAPIDIA_MILEAGE_MAX_SIZE (E2END + 1 - RAPIDIA_MILEAGE_EEPROM_START)

#define RAPIDIA_MILEAGE_SIZE_FULL (RAPIDIA_MILEAGE_SAVE_MULTIPLICITY * (1 + sizeof(Rapidia::MileageData)))

static_assert(RAPIDIA_MILEAGE_SIZE_FULL < RAPIDIA_MILEAGE_MAX_SIZE, "Insufficient room for mileage store in eeprom");

namespace Rapidia
{
  Mileage milage;
  volatile uint16_t Mileage::e_step_tally = 0;
  MileageData Mileage::_data;
  millis_t Mileage::save_interval_ms = SEC_TO_MS(RAPIDIA_MILEAGE_SAVE_INTERVAL);
  millis_t Mileage::next_save_time_ms = 0;

  static bool is_loaded = false;

  void Mileage::update()
  {
    millis_t now = millis();
    if (ELAPSED(now, next_save_time_ms))
    {
      next_save_time_ms = now + save_interval_ms;

      write_eeprom();
    }
  }

  MileageData& Mileage::data()
  {
    if (!is_loaded)
    {
      load_eeprom();
    }

    return _data;
  }

  void Mileage::add_tally()
  {
    // critical section
    cli();
    auto steps = e_step_tally;
    e_step_tally = 0;
    sei();

    #if ENABLED(RAPIDIA_DEV)
    if (steps >= 0x8000)
    {
      SERIAL_ECHO_START();
      SERIAL_ECHOLNPGM("Warning! stepper rate faster than mileage updater can handle. Update Mileage::e_step_tally to uint32_t.");
    }
    #endif
  }

  void Mileage::load_eeprom()
  {
    // TODO

    if (false)
    {
    load_fail:
      SERIAL_ECHO_START();
      SERIAL_ECHOLNPGM("Failed to load Mileage data. Resetting Mileage.");
      memset(&_data, 0, sizeof(_data));
    }
  }

  void Mileage::write_eeprom()
  {
    if (is_loaded)
    {
      // TODO
    }

    // (if not loaded, fail silently)
  }

  void MileageData::update_crc()
  {
    // TODO
  }

  bool MileageData::crc_check()
  {
    // TODO
  }

} // namespace Rapidia

#endif // ENABLED(RAPIDIA_MILEAGE)