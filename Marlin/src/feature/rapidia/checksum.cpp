#include "checksum.h"
#include "../../libs/crc16.h"

#if ENABLED(RAPIDIA_CHECKSUMS)

namespace Rapidia
{
    bool checksums_enabled = false;

    checksum_mode_t checksum_mode { CHECKSUMS_DISABLED };

    // contains a string of the form "*XXXX" for the checksum; can be printed.
    static char checksum_string[7] = {'*'};

    uint8_t& checksum8(checksum_t& checksum)
    {
        return *reinterpret_cast<uint8_t*>(&checksum);
    }

    void checksum(checksum_t& c, const void* data, size_t length)
    {
        switch (checksum_mode)
        {
        case CHECKSUMS_DISABLED:
            return;
        case CHECKSUMS_XOR:
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

    void checksum_pgm(checksum_t& c, const void* data, size_t length)
    {
        while (length --> 0)
        {
            uint8_t v = pgm_read_byte((static_cast<const uint8_t*>(data) + length));
            checksum(c, &v, 1);
        }
    }

    void checksum_echoln(checksum_t& c)
    {
        if (checksum_mode == CHECKSUMS_DISABLED) return;
        itoa(c, checksum_string + 1, (checksum_mode == CHECKSUMS_CRC16) ? 10 : 16);
        SERIAL_ECHOLN(checksum_string);
        c = 0;
    }
}

#endif