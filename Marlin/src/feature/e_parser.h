/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * e_parser.h - Intercept special commands directly in the serial stream
 */

#include "../inc/MarlinConfigPre.h"
#include "../MarlinCore.h"

#if ENABLED(HOST_PROMPT_SUPPORT)
  #include "host_actions.h"
#endif

#if ENABLED(RAPIDIA_PAUSE)
  #include "rapidia/pause.h"
#endif

#if ENABLED(RAPIDIA_DEV)
  #include "../gcode/rapidia/dev/devcode.h"
#endif

// External references
extern bool wait_for_user, wait_for_heatup;
void quickstop_stepper();

#if ENABLED(RAPIDIA_PAUSE) || ENABLED(RAPIDIA_DEV) || ENABLED(RAPIDIA_KILL_RECOVERY)
  // used in this header file only
  #define RAPIDIA_EMERGENCY_PARSER
#endif

class EmergencyParser {

public:

  // Currently looking for: M108, M112, M410, M876
  enum State : char {
    EP_RESET,
    EP_N,
    EP_M,
    EP_M1,
    EP_M10,
    EP_M108,
    EP_M11,
    EP_M112,
    EP_M4,
    EP_M41,
    EP_M410,
    #if ENABLED(HOST_PROMPT_SUPPORT)
      EP_M8,
      EP_M87,
      EP_M876,
      EP_M876S,
      EP_M876SN,
    #endif
    #if ENABLED(RAPIDIA_EMERGENCY_PARSER)
      EP_R,
      EP_R7,
      EP_R75,
      EP_R750,
      EP_R751,
      EP_R752,
      EP_R753,
      EP_R8,
      EP_R80,
      EP_R801,
    #endif
    EP_IGNORE // to '\n'
  };

  static bool killed_by_M112;

  #if ENABLED(HOST_PROMPT_SUPPORT)
    static uint8_t M876_reason;
  #endif

  EmergencyParser() { enable(); }

  FORCE_INLINE static void enable()  { enabled = true; }

  FORCE_INLINE static void disable() { enabled = false; }

  static void on_killed_by_m112();

  FORCE_INLINE static void update(State &state, const uint8_t c) {

    #if ENABLED(RAPIDIA_DEBUG)
      reset_kill_msg[3] = 'E';
    #endif

    #ifdef RAPIDIA_EOT_EMERGENCY_STOP
    if (c == '\4')
    {
      state = EP_M112;
      goto message_complete;
    }
    #endif

    #define ISEOL(C) ((C) == '\n' || (C) == '\r')
    switch (state) {
      case EP_RESET:
        switch (c) {
          case ' ': case '\n': case '\r': break;
          case 'N': state = EP_N;      break;
          case 'M': state = EP_M;      break;
          #if ENABLED(RAPIDIA_EMERGENCY_PARSER)
            case 'R': state = EP_R;      break;
          #endif
          default: state  = EP_IGNORE;
        }
        break;

      case EP_N:
        switch (c) {
          case '0': case '1': case '2':
          case '3': case '4': case '5':
          case '6': case '7': case '8':
          case '9': case '-': case ' ':   break;
          case 'M': state = EP_M;      break;
          #if ENABLED(RAPIDIA_PAUSE)
            case 'R': state = EP_R;    break;
          #endif
          default:  state = EP_IGNORE;
        }
        break;

      case EP_M:
        switch (c) {
          case ' ': break;
          case '1': state = EP_M1;     break;
          case '4': state = EP_M4;     break;
          #if ENABLED(RAPIDIA_PAUSE) && ENABLED(RAPIDIA_M_CODE_COMPATABILITY)
          // Attention! This is a hack. The benefit is having fewer #if ENABLED(...)
          case '7': state = EP_R7;     break;
          #endif
          #if ENABLED(HOST_PROMPT_SUPPORT)
            case '8': state = EP_M8;     break;
          #endif
          default: state  = EP_IGNORE;
        }
        break;

      case EP_M1:
        switch (c) {
          case '0': state = EP_M10;    break;
          case '1': state = EP_M11;    break;
          default: state  = EP_IGNORE;
        }
        break;

      case EP_M10:
        state = (c == '8') ? EP_M108 : EP_IGNORE;
        break;

      case EP_M11:
        state = (c == '2') ? EP_M112 : EP_IGNORE;
        break;

      case EP_M4:
        state = (c == '1') ? EP_M41 : EP_IGNORE;
        break;

      case EP_M41:
        state = (c == '0') ? EP_M410 : EP_IGNORE;
        break;
        
      #if ENABLED(RAPIDIA_EMERGENCY_PARSER)

      case EP_R:
        switch (c)
        {
          case '7': state = EP_R7; break;
          case '8': state = EP_R8; break;
          default: state = EP_IGNORE; break;
        }
        break;

      case EP_R7:
        state = (c == '5') ? EP_R75 : EP_IGNORE;
        break;
        
      case EP_R75:
        switch (c)
        {
          case '0': state = EP_R750; break;
          case '1': state = EP_R751; break;
          case '2': state = EP_R752; break;
          case '3': state = EP_R753; break;
          default: state = EP_IGNORE;
        }
        break;

      case EP_R8:
        state = (c == '0') ? EP_R80 : EP_IGNORE;
        break;

      case EP_R80:
        state = (c == '1') ? EP_R801 : EP_IGNORE;
        break;
      #endif

      #if ENABLED(HOST_PROMPT_SUPPORT)
      case EP_M8:
        state = (c == '7') ? EP_M87 : EP_IGNORE;
        break;

      case EP_M87:
        state = (c == '6') ? EP_M876 : EP_IGNORE;
        break;

      case EP_M876:
        switch (c) {
          case ' ': break;
          case 'S': state = EP_M876S; break;
          default:  state = EP_IGNORE; break;
        }
        break;

      case EP_M876S:
        switch (c) {
          case ' ': break;
          case '0': case '1': case '2':
          case '3': case '4': case '5':
          case '6': case '7': case '8':
          case '9':
            state = EP_M876SN;
            M876_reason = (uint8_t)(c - '0');
            break;
        }
        break;
      #endif

      case EP_IGNORE:
        if (ISEOL(c)) state = EP_RESET;
        break;

      default:
        if (ISEOL(c)) {
          message_complete:
          if (enabled) switch (state) {
            case EP_M108: wait_for_user = wait_for_heatup = false; break;
            case EP_M112: killed_by_M112 = true; break;
            case EP_M410: 
              if (marlin_state != MF_KILLED)
              {
                quickstop_stepper();
              }
              break;
            #if ENABLED(RAPIDIA_KILL_RECOVERY)
              case EP_R750: hard_reset();
            #endif
            #if ENABLED(RAPIDIA_PAUSE)
              case EP_R751: Rapidia::pause.defer(false); break;
              case EP_R752: Rapidia::pause.defer(true); break;
            #endif
            #if ENABLED(RAPIDIA_KILL_RECOVERY)
              // FIXME: it's unclear why, but R753
              // doesn't seem to run from the e-parser properly;
              // instead it is dispatched from gcode.cpp.
              case EP_R753: hard_reset_bl(); break;
            #endif
            #if ENABLED(RAPIDIA_DEV)
              case EP_R801: Rapidia::R801(); break;
            #endif
            #if ENABLED(HOST_PROMPT_SUPPORT)
              case EP_M876SN: host_response_handler(M876_reason); break;
            #endif
            default: break;
          }
          state = EP_RESET;
        }
    }
  }

private:
  static bool enabled;
};

extern EmergencyParser emergency_parser;