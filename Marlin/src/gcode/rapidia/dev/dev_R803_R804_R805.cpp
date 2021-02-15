#include "../../../inc/MarlinConfig.h"
#include "../../gcode.h"
#include "../../../feature/rapidia/checksum.h"
#include "../../../HAL/shared/eeprom_api.h"
#include "../../../MarlinCore.h"
#include "../../../HAL/HAL.h"
#include "../../../feature/rapidia/stack_util.h"

#if ENABLED(RAPIDIA_DEV)

#define ROW_WIDTH 0x10

// read EEPROM
void GcodeSuite::R803()
{
    SERIAL_INIT_CHECKSUM();

    uint16_t length = 1;
    if (parser.seenval('L'))
    {
        length = parser.value_ushort();
    }

    if (parser.seenval('O'))
    {
        const uint16_t offset = parser.value_ushort();
        const uint16_t disp_start = offset;
        const uint16_t disp_end = offset + length;

        // round start down and end up.
        uint16_t start = (disp_start / ROW_WIDTH) * ROW_WIDTH;
        uint16_t end = ((disp_end + ROW_WIDTH - 1) / ROW_WIDTH) * ROW_WIDTH;
        end = min(end, E2END + 1);

        for (uint16_t i = start; i < end; ++i)
        {
            if (i % ROW_WIDTH == 0)
            {
                // row header
                SERIAL_ECHOPGM_CHK("EEPROM ");
                serial_hex_chk(CHK_ARGS i, 3);
                SERIAL_ECHOPGM_CHK(":");
            }

            SERIAL_CHAR_CHK(' ');

            if (i >= disp_start && i < disp_end)
            {
                // display hex value of 
                uint8_t v;
                persistentStore.read(i, v);
                serial_hex_chk(CHK_ARGS v, 2);
            }
            else
            {
                // display two dots
                SERIAL_CHAR_CHK('.');
                SERIAL_CHAR_CHK('.');
            }
            
            if (i % ROW_WIDTH == ROW_WIDTH - 1)
            {
                SERIAL_EOL_CHK();
            }
        }
    }
}

// write EEPROM
void GcodeSuite::R804()
{
    if (parser.seenval('O'))
    {
        const uint16_t offset = parser.value_ushort();
        if (parser.seenval('V'))
        {
            const uint8_t value = parser.value_byte();
            if (persistentStore.write(offset, value))
            {
                SERIAL_ERROR_MSG("EEPROM -- write fail.");
            }
        }
    }
}

// EEPROM scan -- incurs 2 writes on each entry (or more if errors detected)
void GcodeSuite::R805()
{
    #define chunk_size 0x100
    #define chunk8_size (chunk_size * 8)
    #define chunk_count (E2END + 1 + chunk_size - 1) / chunk_size
    #define chunk8_count (chunk_count + 7) / 8

    union _local_union_t {
        struct {
            uint16_t _crcs[chunk_count];
            uint16_t _crcs_cmp[chunk_count];
        } _a;
        struct {
            bool _chunk_err[chunk_size];
            uint8_t _chunkstore[chunk_size];
        } _b;
    };

    // 0x150 is chosen arbitrarily but is probably enough for the ISRs to interrupt and still have space.
    if (Rapidia::get_stack_remaining() < 0x150 + sizeof(_local_union_t) + chunk8_count)
    {
        SERIAL_ECHO_MSG("Insufficient stack space to perform R805.");
        return;
    }

    // stack-allocated local variables
    uint8_t crcs_err[chunk8_count];
    _local_union_t _union;

    #define crcs _union._a._crcs
    #define crcs_cmp _union._a._crcs_cmp
    #define chunk_err _union._b._chunk_err
    #define chunkstore _union._b._chunkstore

    //! -------------------------------------------------------------------
    //! active union member: __c

    memset(crcs, 0, sizeof(crcs));
    memset(crcs_err, 0, sizeof(crcs_err));

    #define set_crc_err(i) (crcs_err[(i) / chunk8_size] |= _BV(((i) / chunk_size) % 8))
    #define get_crc_err(i) (crcs_err[(i) / chunk8_size] & _BV(((i) / chunk_size) % 8))

    // calculate crc of each chunk and flip every bit.
    for (size_t i = 0; i < E2END + 1; ++i)
    {
        uint16_t& crc = crcs[i / chunk_size];
        uint8_t v;
        persistentStore.read(i, v);
        crc16(&crc, &v, 1);

        // flip digit
        v = ~v;
        if (persistentStore.write(i, v))
        {
            set_crc_err(i);
        }
    }

    // crc check
    for (size_t k = 0; k < 2; ++k)
    {
        memset(crcs_cmp, 0, sizeof(crcs_cmp));

        idle();

        // re-calculate crc of each chunk, and return to the normal value.
        for (size_t i = 0; i < E2END + 1; ++i)
        {
            uint16_t& crc = crcs_cmp[i / chunk_size];
            uint8_t v;
            persistentStore.read(i, v);
            if (k == 0)
            {
                v = ~v;
                if (persistentStore.write(i, v))
                {
                    set_crc_err(i);
                }
            }
            crc16(&crc, &v, 1);
        }

        // set errror bit for crcs
        for (size_t i = 0; i < chunk_count; ++i)
        {
            if (crcs[i] != crcs_cmp[i]) set_crc_err(i * chunk_size);
        }
    }

    idle();

    //! -------------------------------------------------------------------
    //! active union member is: __d

    #define SCANH "EEPROM Scan: "

    SERIAL_INIT_CHECKSUM();

    // go through chunks, identify error regions
    uint32_t error_count = 0;
    for (size_t i = 0; i < chunk_count; ++i)
    {
        if (!get_crc_err(i * chunk_size))
        {
            SERIAL_ECHOPGM_CHK(SCANH "bytes ");
            serial_hex_chk(CHK_ARGS (i * chunk_size), 3);
            SERIAL_CHAR_CHK('-');
            serial_hex_chk(CHK_ARGS (min(E2END, i * chunk_size + chunk_size - 1)), 3); // subtract 1 because inclusive bound.
            SERIAL_ECHOLNPGM_CHK(" ok.");
        }
        else
        {
            // error in this chunk.
            memset(chunk_err, 0, sizeof(chunk_err));
            
            for (size_t j = 0; j < chunk_size; ++j)
            {
                uint16_t idx = i * chunk_size + j;
                if (idx > E2END) break;
                persistentStore.read(idx, chunkstore[j]);
                chunkstore[j] = ~chunkstore[j];
                uint8_t v = chunkstore[j];
                if (persistentStore.write(idx, v))
                {
                    chunk_err[j] = true;
                }
            }

            idle();
            
            bool ok_rle;
            uint16_t rle_from = i * chunk_size;
            for (size_t j = 0; j < chunk_size; ++j)
            {
                const uint16_t idx = i * chunk_size + j;
                uint8_t v;
                persistentStore.read(idx, v);
                const bool ok = (chunkstore[j] == v) && !chunk_err[j];
                error_count += !ok;
                const bool end = (idx == E2END || j == chunk_size - 1);

                if (ok != ok_rle || end)
                {
                    const uint16_t rle_end = idx + (ok == ok_rle);
                    if (rle_from != rle_end)
                    {
                        if (rle_end - 1 == rle_from)
                        {
                            SERIAL_ECHOPGM_CHK(SCANH "byte ");
                            serial_hex_chk(CHK_ARGS rle_from, 3);
                        }
                        else
                        {
                            SERIAL_ECHOPGM_CHK(SCANH "bytes ");
                            serial_hex_chk(CHK_ARGS rle_from, 3);
                            SERIAL_CHAR_CHK('-');
                            serial_hex_chk(CHK_ARGS rle_end - 1, 3); // subtract 1 because inclusive bound.
                        }
                        if (rle_from % chunk_size == 0 && j == chunk_size - 1 && ok)
                        {
                            // crc check failed, but we couldn't identify any damaged bits.
                            SERIAL_ECHOLNPGM_CHK(" unsure.");
                        }
                        else
                        {
                            if (ok_rle)
                            {
                                SERIAL_ECHOLNPGM_CHK(" ok.");
                            }
                            else
                            {
                                SERIAL_ECHOLNPGM_CHK(" damaged.");
                            }
                        }
                    }
                    ok_rle = ok;
                    rle_from = idx;
                }

                if (end) break;
            }
        }
    }

    SERIAL_ECHOPGM_CHK(SCANH "complete. 0x");
    serial_hex_chk(CHK_ARGS error_count, 4);
    SERIAL_ECHOPGM_CHK(" of 0x");
    serial_hex_chk(CHK_ARGS E2END + 1, 4);
    SERIAL_ECHOLNPGM_CHK(" bytes damaged.");
}
#endif // RAPIDIA_DEV

