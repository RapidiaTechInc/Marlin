#pragma once

#include "../../inc/MarlinConfigPre.h"
#include "../../core/serial.h"
#include <stddef.h>

#if ENABLED(RAPIDIA_CHECKSUMS)

namespace Rapidia
{
    // enum for how to checksum outgoing transmissions.
    extern enum checksum_mode_t {
        CHECKSUMS_DISABLED,
        CHECKSUMS_XOR,
        CHECKSUMS_XOR_OPTIONAL,
        CHECKSUMS_CRC16
    } checksum_mode_out, checksum_mode_in;

    typedef uint16_t checksum_t;

    void checksum(checksum_t& checksum, const void* data, size_t length, checksum_mode_t=checksum_mode_out);
    void checksum_pgm(checksum_t& checksum, const void* data, size_t length, checksum_mode_t=checksum_mode_out);

    // echoes "*XXXX\n" then resets checksum to 0.
    void checksum_eol(checksum_t& checksum, checksum_mode_t=checksum_mode_out);

    // compares checksum against the given string
    // returns true on error.
    bool compare_checksum(checksum_t checksum, const char* compare, checksum_mode_t);

    inline bool checksum_required(checksum_mode_t c)
    {
        switch (c)
        {
        case CHECKSUMS_DISABLED:
        case CHECKSUMS_XOR_OPTIONAL:
            return false;
        default:
            return true;
        }
    }
}

#define SERIAL_INIT_CHECKSUM() Rapidia::checksum_t __crc__ = 0
#define _SERIAL_FN_CHECKSUM_(str) \
    do { const char* _s = str; Rapidia::checksum(__crc__, _s, strlen(_s)); SERIAL_OUT(print, _s); } while (0)
#define _SERIAL_FN_PGM_CHECKSUM_(str) \
    do { const char* _s = PSTR(str); Rapidia::checksum_pgm(__crc__, _s, strlen(_s)); serialprintPGM(_s); } while (0)
#define SERIAL_ECHO_START_CHK() SERIAL_ECHOPGM_CHK("echo:")
#define SERIAL_ECHO_CHK(str) _SERIAL_FN_CHECKSUM_(str)
#define SERIAL_ECHOPGM_CHK(str) _SERIAL_FN_PGM_CHECKSUM_(str)
#define SERIAL_ECHOLN_CHK(str) do { _SERIAL_FN_CHECKSUM_(str); Rapidia::checksum_eol(__crc__); } while (0)
#define SERIAL_ECHOLNPGM_CHK(str) do { _SERIAL_FN_PGM_CHECKSUM_(str); Rapidia::checksum_eol(__crc__); } while (0)
#define SERIAL_EOL_CHK() do { Rapidia::checksum_eol(__crc__); } while (0)
#define SERIAL_CHAR_CHK(c) do { char _str[2]; _str[0] = c; _str[1] = 0; SERIAL_ECHO_CHK(_str); } while (0)
#define ECHO_KEY_CHK(c) do { char _str[] = { '"', c, '"', ':', 0 }; SERIAL_ECHO_CHK(_str); } while (0)
#define ECHO_KEY_STR_CHK(c) do { SERIAL_CHAR_CHK('"'); SERIAL_ECHO_CHK(c); SERIAL_CHAR_CHK('"'); SERIAL_CHAR_CHK(':'); } while (0)
#define CHK_ARGDEF Rapidia::checksum_t& __crc__
#define CHK_ARG __crc__
#define CHK_ARGSDEF CHK_ARGDEF,
#define CHK_ARGS CHK_ARG,

#else

#define SERIAL_INIT_CHECKSUM()
#define SERIAL_ECHO_START_CHK() SERIAL_ECHO_START()
#define SERIAL_ECHO_CHK(str) SERIAL_ECHO(str)
#define SERIAL_ECHOPGM_CHK(str) SERIAL_ECHOPGM(str)
#define SERIAL_ECHOLN_CHK(str) SERIAL_ECHOLN(str)
#define SERIAL_ECHOLNPGM_CHK(str) SERIAL_ECHOLNPGM(str)
#define SERIAL_CHAR_CHK(c) SERIAL_CHAR(c)
#define ECHO_KEY_CHK(c) echo_key(c)
#define ECHO_KEY_STR_CHK(c) echo_key_str(c)
#define SERIAL_EOL_CHK() SERIAL_EOL()
#define CHK_ARGDEF
#define CHK_ARG
#define CHK_ARGSDEF
#define CHK_ARGS

#endif

#define ECHO_SEPARATOR_CHK(io_first_separator) do { if (!io_first_separator) SERIAL_CHAR_CHK(','); io_first_separator = false; } while (0)