#include "mileage.h"

#include "../../core/serial.h"
#include "../../HAL/shared/eeprom_api.h"
#include "../../libs/crc16.h"
#include "../../MarlinCore.h"

#include <avr/interrupt.h>

#if ENABLED(RAPIDIA_MILEAGE)

namespace Rapidia
{
  // (chosen arbitrarily)
  #define CANARY_1 0xde8bfaed
  #define CANARY_2 0xb8fd7fa1

  struct MileageHeader
  {
    uint32_t canary_1;
    uint8_t save_index;
    uint8_t save_index_xor;
    uint8_t reserved[32];
    uint32_t canary_2;
    uint16_t crc;

    // return false if no error.
    bool is_valid()
    {
      if (canary_1 != CANARY_1) return false;
      if (canary_2 != CANARY_2) return false;
      uint16_t _c = 0;
      crc16(&_c, this, sizeof(this) - sizeof(crc));
      if (_c != crc) return false;
      return (save_index_xor^save_index != 0xff);
    }

    bool make_valid()
    {
      canary_1 = CANARY_1;
      canary_2 = CANARY_2;
      save_index_xor = ~save_index;
      memset(reserved, 0, sizeof(reserved));
      crc = 0;
      crc16(&crc, this, sizeof(this) - sizeof(crc));
    }
  };

#define MILEAGE_HEADER_START RAPIDIA_MILEAGE_EEPROM_START
#define MILEAGE_HEADER_SIZE sizeof(Rapidia::MileageHeader)
#define MILEAGE_DATA_START (RAPIDIA_MILEAGE_EEPROM_START + MILEAGE_HEADER_SIZE)

#define RAPIDIA_MILEAGE_MAX_SIZE (E2END + 1 - RAPIDIA_MILEAGE_EEPROM_START)
#define RAPIDIA_MILEAGE_SIZE_FULL (RAPIDIA_MILEAGE_SAVE_MULTIPLICITY * (sizeof(Rapidia::MileageData)) + MILEAGE_HEADER_SIZE)

static_assert(RAPIDIA_MILEAGE_SIZE_FULL < RAPIDIA_MILEAGE_MAX_SIZE, "Insufficient room for mileage store in eeprom");

Mileage milage;
decltype(Mileage::e_step_tally) Mileage::e_step_tally{ 0 };
MileageData Mileage::_data;
millis_t Mileage::save_interval_ms = SEC_TO_MS(RAPIDIA_MILEAGE_SAVE_INTERVAL);
millis_t Mileage::next_save_time_ms = 0;

// TODO: make these class members (for neatness)
static bool is_loaded = false;
static bool is_expended = false;
static size_t save_index = 0;

bool memeq(void* v, uint8_t val, size_t length)
{
  while (length --> 0)
  {
    if (*reinterpret_cast<uint8_t*>(v) != val) return false;
  }

  return true;
}

void Mileage::update()
{
  add_tally();

  millis_t now = millis();
  if (ELAPSED(now, next_save_time_ms) && is_loaded)
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
    SERIAL_ECHOLNPGM("Warning! Stepper rate faster than mileage cache can process. Update Mileage::e_step_tally size.");
  }
  #endif

  data().e_steps += steps;
}

// these are chosen arbitrarily.
#define INDEX_NULL_BYTE 0x59
#define INDEX_SET_BYTE 0xc2

bool Mileage::header_is_empty()
{
  uint8_t buff[sizeof(MileageHeader)];
  if (memeq(buff, 0xff, sizeof(MileageHeader))) return true;
  if (memeq(buff, 0, sizeof(MileageHeader))) return true;
  return false;
}

bool Mileage::read_header()
{
  MileageHeader hdr;

  if (persistentStore.read(MILEAGE_HEADER_START, hdr)) return true;
  if (!hdr.is_valid()) return true;

  save_index = hdr.save_index;

  return false;
}

bool Mileage::write_header()
{
  MileageHeader hdr {
    save_index: save_index
  };

  hdr.make_valid();
  assert_kill(hdr.is_valid());
  return persistentStore.write(MILEAGE_HEADER_START, hdr);
}

bool Mileage::load_eeprom()
{
  if (!is_loaded)
  {
    if (header_is_empty()) return load_fail(ErrorCode::FIRST_TIME);

    if (read_header()) return load_fail(ErrorCode::FORMAT);
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
    int pos = (MILEAGE_DATA_START + save_index * sizeof(MileageData));
    if (persistentStore.write(pos, _data)) continue;

    // read and check CRC
    MileageData check;
    if (persistentStore.read(pos, check)) continue;
    if (check.crc_check()) continue;

    // we've found a spot for the data, so let's write the header.
    if (write_header()) return save_fail(ErrorCode::HEADER_DAMAGE);

    // success!
    return false;
  }

  // we've run out of slots.
  assert_kill(save_index >= RAPIDIA_MILEAGE_SAVE_MULTIPLICITY);
  write_header(); // write the out-of-bounds save index so next time we load we can recognize that the eeprom is spent.
  return save_fail(ErrorCode::EXPENDED);
}

void Mileage::fail(ErrorCode e)
{
  switch(e)
  {
    case ErrorCode::CRC_MISMATCH:
      SERIAL_ECHOPGM("checksum failed");
      break;
    case ErrorCode::EXPENDED:
      // this can really be interpreted more like "undamaged EEPROM depleted,"
      // since the mileage data moves to undamaged EEPROM regions.
      SERIAL_ECHOPGM("EEPROM damaged");
      is_expended = true;
      break;
    case ErrorCode::HEADER_DAMAGE:
      // this can really be interpreted more like "undamaged EEPROM depleted,"
      // since the mileage data moves to undamaged EEPROM regions.
      SERIAL_ECHOPGM("EEPROM header damaged");
      is_expended = true;
      break;
    case ErrorCode::FORMAT:
      SERIAL_ECHOPGM("invalid format");
      break;
    case ErrorCode::FIRST_TIME:
     SERIAL_ECHOPGM("No existing mileage data found");
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
  if (e != ErrorCode::FIRST_TIME) SERIAL_ECHOPGM("Warning! failed to recover mileage data: ");
  fail(e);
  SERIAL_ECHOLNPGM(". Resetting mileage.");

  reset();

  return true;
}

void Mileage::reset(bool save)
{
  _data.e_steps = 0;
  is_loaded = true;
  is_expended = false;
  save_index = 0;

  // reset save timer too (even if we don't save now).
  next_save_time_ms = millis() + save_interval_ms;

  if (save)
  {
    save_eeprom();
  }
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