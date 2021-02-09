#include "mileage.h"

#include "../../core/serial.h"
#include "../../HAL/shared/eeprom_api.h"
#include "../../libs/crc16.h"
#include "../../MarlinCore.h"

#include <avr/interrupt.h>

#if ENABLED(RAPIDIA_MILEAGE)

#define RAPIDIA_MILEAGE_MAX_SIZE (E2END + 1 - RAPIDIA_MILEAGE_EEPROM_START)

#define RAPIDIA_MILEAGE_SIZE_FULL (RAPIDIA_MILEAGE_SAVE_MULTIPLICITY * (1 + sizeof(Rapidia::MileageData)))

#define MILEAGE_HEADER_START RAPIDIA_MILEAGE_EEPROM_START
#define MILEAGE_DATA_START (RAPIDIA_MILEAGE_EEPROM_START + RAPIDIA_MILEAGE_SAVE_MULTIPLICITY)

static_assert(RAPIDIA_MILEAGE_SIZE_FULL < RAPIDIA_MILEAGE_MAX_SIZE, "Insufficient room for mileage store in eeprom");

namespace Rapidia
{
  Mileage milage;
  decltype(Mileage::e_step_tally) Mileage::e_step_tally{ 0 };
  MileageData Mileage::_data;
  millis_t Mileage::save_interval_ms = SEC_TO_MS(RAPIDIA_MILEAGE_SAVE_INTERVAL);
  millis_t Mileage::next_save_time_ms = 0;

  // TODO: make these class members (for neatness)
  static bool is_loaded = false;
  static bool is_expended = false;
  static size_t save_index = 0;

  void Mileage::update()
  {
    add_tally();

    millis_t now = millis();
    if (ELAPSED(now, next_save_time_ms))
    {
      next_save_time_ms = now + save_interval_ms;

      save_eeprom();
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
    decltype(e_step_tally.v) steps = 0;
    cli();
    steps = e_step_tally.v;
    e_step_tally.v = 0;
    sei();

    #if ENABLED(RAPIDIA_DEV)
    if (steps >= 0x800000)
    {
      SERIAL_ECHO_START();
      SERIAL_ECHOLNPGM("Warning! stepper rate faster than mileage cache can process. Update Mileage::e_step_tally to uint32_t.");
    }
    #endif

    data().e_steps += steps;
  }

  // these are chosen arbitrarily.
  #define INDEX_NULL_BYTE 0x59
  #define INDEX_SET_BYTE 0xc2

  bool Mileage::read_header()
  {
    // find save index
    size_t index;
    for (index = RAPIDIA_MILEAGE_SAVE_MULTIPLICITY; index --> 0; )
    {
      uint8_t v;
      // (read_data always succeeds on AVR)
      persistentStore.read_data(index + MILEAGE_HEADER_START, &v);
      if (v == INDEX_SET_BYTE)
      {
        // index found.
        break;
      }
      else if (v != INDEX_NULL_BYTE)
      {
        // incorrect format for mileage (or save error)
        return load_fail(ErrorCode::FORMAT);
      }
    }
    save_index = index;

    return false;
  }

  bool Mileage::write_header()
  {
    size_t index;
    for (index = RAPIDIA_MILEAGE_SAVE_MULTIPLICITY; index --> save_index; )
    {
      uint8_t v = (index == save_index)
        ? INDEX_SET_BYTE
        : INDEX_NULL_BYTE;
      if (persistentStore.write(index + MILEAGE_HEADER_START, v))
      {
        // error.
        return true;
      }
    }

    return false;
  }

  bool Mileage::load_eeprom()
  {
    if (!is_loaded)
    {
      if (read_header()) return true;
    }

    // save_index past end -- no recovery from this.
    if (save_index >= RAPIDIA_MILEAGE_SAVE_MULTIPLICITY)
    {
      return load_fail(ErrorCode::EXPENDED);
    }

    persistentStore.read(MILEAGE_DATA_START + save_index * sizeof(MileageData), _data);
    if (!_data.crc_check())
    {
      return load_fail(ErrorCode::CRC_MISMATCH);
    }

    return false;
  }

  bool Mileage::save_eeprom()
  {
    // (if not loaded, fail silently)
    if (!is_loaded)
    {
      return false;
    }

    _data.update_crc();
    assert_kill(!_data.crc_check());

    // try saving in subsequent slots until one works.
    for (; save_index < RAPIDIA_MILEAGE_SAVE_MULTIPLICITY; ++save_index)
    {
      if (write_header()) continue; // (if this fails, try again.)
      
      // write data:
      
      int pos = (MILEAGE_DATA_START + save_index * sizeof(MileageData);
      if (persistentStore.write(pos, _data)) continue;

      // read and check CRC
      MileageData check;
      if (persistentStore.read(pos, check)) continue;
      if (check.crc_check()) continue;
    }

    // we've run out of slots.
    return save_fail(ErrorCode::EXPENDED);
  }

  void Mileage::fail(ErrorCode e)
  {
    switch(e)
    {
      case ErrorCode::CRC_MISMATCH:
        SERIAL_ECHO("checksum failed");
        break;
      case ErrorCode::EXPENDED:
        // this can really be interpreted more like "undamaged SRAM depleted,"
        // since the mileage data moves to undamaged SRAM regions.
        SERIAL_ECHO("SRAM damaged");
        is_expended = true;
        break;
      case ErrorCode::FORMAT:
        SERIAL_ECHO("invalid format");
        break;
    }
  }

  bool Mileage::save_fail(ErrorCode e)
  {
    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("Warning! failed to write mileage data: ");
    fail(e);    
    SERIAL_ECHOLNPGM(".");

    return true;
  }

  bool Mileage::load_fail(ErrorCode e)
  {
    SERIAL_ECHO_START();
    SERIAL_ECHOPGM("Warning! failed to recover mileage data: ");
    fail(e);
    SERIAL_ECHOLNPGM(". Resetting mileage.");
    memset(&_data, 0, sizeof(_data));

    return true;
  }

  uint16_t MileageData::calc_crc()
  {
    uint16_t z = 0;
    crc16(&z, this, sizeof(*this) - sizeof(crc));
    return z;
  }

  void MileageData::update_crc()
  {
    crc = calc_crc();
  }

  bool MileageData::crc_check()
  {
    return calc_crc() == crc;
  }

} // namespace Rapidia

#endif // ENABLED(RAPIDIA_MILEAGE)