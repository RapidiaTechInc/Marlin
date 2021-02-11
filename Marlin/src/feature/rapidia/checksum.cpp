#include "checksum.h"
#include "../../libs/crc16.h"

#if ENABLED(RAPIDIA_CHECKSUMS)

namespace Rapidia
{
    bool checksums_enabled = false;

    checksum_mode_t checksum_mode_out { CHECKSUMS_DISABLED };
    checksum_mode_t checksum_mode_in  { CHECKSUMS_XOR_OPTIONAL };

    // contains a string of the form "*XXXX" for the checksum; can be printed.
    static char checksum_string[7] = {'*'};

    uint8_t& checksum8(checksum_t& checksum)
    {
        return *reinterpret_cast<uint8_t*>(&checksum);
    }

    void checksum(checksum_t& c, const void* data, size_t length, checksum_mode_t checksum_mode)
    {
        switch (checksum_mode)
        {
        case CHECKSUMS_DISABLED:
            return;
        case CHECKSUMS_XOR:
        case CHECKSUMS_XOR_OPTIONAL:
            while (length --> 0)
            {
                checksum8(c) ^= *(static_cast<const uint8_t*>(data) + length);
            }
            return;
        case CHECKSUMS_CRC16:
            crc16(&c, data, length);
            return;
        }
    }

    void checksum_pgm(checksum_t& c, const void* data, size_t length, checksum_mode_t checksum_mode)
    {
        while (length --> 0)
        {
            uint8_t v = pgm_read_byte((static_cast<const uint8_t*>(data) + length));
            checksum(c, &v, 1, checksum_mode);
        }
    }

    void checksum_eol(checksum_t& c, checksum_mode_t checksum_mode)
    {
        if (checksum_mode == CHECKSUMS_DISABLED)
        {
            SERIAL_EOL();
            return;
        }
        
        // the value to print
        uint32_t v = (checksum_mode == CHECKSUMS_XOR || checksum_mode == CHECKSUMS_XOR_OPTIONAL)
            ? checksum8(c)
            : c;

        // radix to print with (decimal or hex)
        int radix = (checksum_mode == CHECKSUMS_CRC16)
            ? 16
            : 10;
            
        // place string after '*' character.
        itoa(v, checksum_string + 1, radix);
        SERIAL_ECHOLN(checksum_string);

        // reset checksum for next line.
        c = 0;
    }

    bool compare_checksum(checksum_t c, const char* cmp, checksum_mode_t checksum_mode)
    {
        switch (checksum_mode)
        {
        case CHECKSUMS_DISABLED:
            // checksums disabled means checksums ignored.
            return false;
        case CHECKSUMS_XOR:
        case CHECKSUMS_XOR_OPTIONAL:
            return strtol(cmp, nullptr, 10) != checksum8(c);
        case CHECKSUMS_CRC16:
            return strtol(cmp, nullptr, 16) != c;
        default:
            // (unsupported checksum mode.)
            return true;
        }
    }
}

#endif