# Rapidia 3D Printer Firmware

Marlin is an open source project which drives many of the world's 3D printers. It is considered to be a reliable and professional-grade firwmare project. Additional documentation can be found at the [Marlin Home Page](http://marlinfw.org/).

Rapidia has made some modifications to Marlin to work better with our hardware and host software. Because this project is licensed under [version 3.0 of the GNU General Public License](./LICENSE), the source code and license must be made available with every distribution of the Marlin firmware.

Most Rapidia-specific changes to the code can be identified by the surrounding RAPIDIA\_\* macros. A list of the new gcode commands created for and existing commands modified for Rapidia use has been provided below.

## Build and Upload with PlatformIO

You can download the [PlatformIO extension for VSCode](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) (recommended) or the [PlatformIO CLI](https://platformio.org/install/cli) tool to build and upload the firmware.

On linux, the user must be added to the `dialout` group:

```
sudo usermod -a -G dialout $USER
```

### VSCode

Click on the PlatformIO icon on the toolbar on the left, then use Default > General > Upload All. (Despite the name, only one configuration will be built.)

### PlatformIO CLI

In BASH:

```
pio run -e rapidia_export -t upload
```

### PlatformIO Configurations

#### (Default)

Basic build for metal printer.

#### rapidia_export

Copies files to Rapidia Host (which must be located adjacent to Marlin in the file system), then commits and pushes a change to Rapidia Host.

#### rapidia_plastic

Port of Rapidia Marlin to standard plastic printers, for testing purposes.

#### rapidia_arduino

Removes various reliances on peripherals (such as thermistors) so that the firmware can be run
on a stock arduino with no peripherals attached (for testing purposes).

## Build with Docker

- [Install Docker](https://docs.docker.com/get-docker/)
- Run `docker build . --tag arduino-cli`
- Run `docker run --rm -v ${PWD}:/Marlin -w /Marlin arduino-cli:latest /bin/sh ./arduino-build.sh`

## Changes

### M155 [S(u8:seconds)] *

temperature setting. This exists in the original firmware, but now additional args will be passed to R738, allowing setting
heartbeat and temperature simultaneously. See below.

### R710 T(0-3)

Set Timer. There are 4 timers which can be used in conditional gcode execution (see R711). This command resets the specified timer.

### R711 T(0-3) [A(u32)][b(u32)] [N(u8)]

Conditional Execution (on Timer). This command checks if the specified timer T has elapsed at least (A) or fewer than (B) the specified number of milliseconds. If it has not -- that is, if the condition evaluates to false -- then the next specified number N lines of gcode received will be skipped. Only gcode which starts with a ‘G’ or ‘T’ will be skipped; ‘M’ code will neither be skipped nor will it count toward the number of lines which are to be skipped.

### R730; R731

Enable/Disable movement-complete auto-reporting.
These respectively enable and disable reporting when individual lines of gcode have completed execution. (Note that this does not enable synchronous movement.)

### R732 [I,O(0,1,2,3)]

Set serial communication checksum mode. Input (`I`) and output (`O`) checksum modes can be set independently.

Flags:

- `I`: Set input checksum mode (default: 1); for gcode checksums.
- `O`: Set output checksum mode (default: 0). Note: not all output messages support checksums (yet).
     (As of writing, only heartbeat messages (R738/R739) support checksums)

Values:

- `0`: No checksum.
- `1`: 8-bit parity check -- computes the XOR of each byte. Reported at the end of the line with `*` followed by 3 digits decimal.
- `2`: Optional 8-bit parity check -- as above, but unlike other options, a missing checksum will not be treated as an error.
- `3`: CRC16/XMODEM (Note carefully that the XMODEM version of CRC16 is used) -- reported at the end of the line with `*` followed by 4-digits hexadecimal.

Example command:
`R732 O2`

Example checksums:
- mode `0`: ` Message `
- mode `1`/`2`: ` Message *75`
- mode `3`: ` Message *54FD`

Note for input only: leading spaces are skipped and are not included in the checksum.

### R733

*[Dev code]*

Pin test.

Directly pulses various pins. This will cause the firmware to forget which pins are currently HIGH and LOW, so only use this for testing purposes, never in the context of an actual print job.

### R735 [S(u8)][i(u16:milliseconds)]

Z_MAX (nozzle plug) signal processing.

Arguments:

- S: sample threshold.
- I: minimum sample interval.

Setting S to an integer greater than 1 makes it so that Marlin must read S positive reads on the Z_MAX endstop pin (i.e. the nozzle plug pin) in a row for it to properly count as triggered. By default, it will sample no faster than once per ms. Setting I to a value greater than 1 means that the endstop will be sampled (no faster than) every I milliseconds.

Warning: setting S0 means that Marlin requires 0 positive reads for the endstop to count as “triggered". In other words, the endstop will always be triggered. This is likely to be useful only for debugging nozzle plug detection.

### R736; R737

Lamp on/Lamp off.
For now, these commands are aliases of M106 and M107.


### R738 [H(s32:milliseconds)] [A,P,C,R,X,E,D(0,1)]

Auto-reporting. H sets the interval at which the heartbeat status update occurs. Temperature and heartbeat reports occur separately, but they are both enabled by this command. P,C,R, etc. can enable/disable individual status updates in that heartbeat. Some of these options are disabled by default (\*). The report is issued as a json object and can contain the following entries:

- P: current plan position. (After executing all the plans in the buffer, - the toolhead will be here.) Also displays key "H", homed status, a string of lower-case letters indicating which axes are homed; an 'h' in this string indicates a homing routine is currently running.
- C: actual current position. (May not be very useful for logic, but could be nifty for UI reasons.)
- F: feedrate shown with plan position.
- R: per-axis relative mode flag enabled/disabled. (Reported as a string containing the axes in relative mode, e.g. “XYZ")
- X\*: dualx state
- E: Endstops states. Reported as a string: endstop state for X_MIN through Z_MIN (reported as ‘x’, ‘y’, ‘z’ in lower case), and X_MAX through Z_MAX (reported as ‘X’, ‘Y’, ‘Z’ in upper case)
- M: Mileage data. Reported as (a) `null`, if mileage is disabled, or (b) an object containing the keys "E0" etc. with the number of steps taken on the E axis per extruder. Also contains key "I", whose value is the current "mileage save index"; if this value equals or exceeds RAPIDIA_MILEAGE_SAVE_MULTIPLICITY (as defined in the firmware), then the EEPROM store for the mileage data is expended.
- D: debug info.
- A: Use `A0` to set all flags to 0, or `A1` to set all flags to the default values, or `A2` to set all flags to on. (This is applied before any of the other flags.)


Example command:
`M155 S3 H2000 A0 P1 C0 R1 X0`

**Example report [H]**

```
H:{"P":{"X":113.925,"Y":100.000,"Z":2.000,"E":0.000,"F":33.333,"T":0},"H":"xyh","C":{"X":113.925,"Y":100.000,"Z":2.000,"E":0.000,"T":0},"R":"","ES":"xXZ"}
```

Note that the “F" and “T" entries in the position object refer to the current feedrate and tool respectively.

### R739 [A,P,C,R,X,E(0,1)]

As above, but sends a heartbeat message immediately upon execution (rather than scheduling a heartbeat interval).
By default, the flags are the same as has been configured with R736, and additional flags specified will modify only
this heartbeat message.

Example commands:
`M155`,
`M155 A0 C1 P1`

### R740

Check for USB shield.

This command can be used to try checking for a connected USB device if
Marlin previously failed to detect it on init.

### R741

Reset Mileage Data.

This resets the ongoing count of the number of E steps taken.

### R742

Save Mileage Data.

While the mileage data is saved to EEPROM roughly every minute or two, this command causes the mileage to be saved immediately. It's recommended to use this command after a print completes, when pausing a print, before any action which is unusually likely to be interrupted by a complete power-down of the printer, and perhaps at the end of every layer.

### R743 I(u16:seconds)

Set Mileage save interval.

The mileage data saves to EEPROM every minute or two by default. This sets the maximum save interval in seconds.

(Note that the EEPROM will not be redundantly overwritten if the mileage data has not changed, i.e. if no extrusion has occurred. There are a limited number of EEPROM writes available.)

### R744 E(u8:extruder) V(u64:amount) S(bool:save)

Directly edit mileage.

Parameters:

- E: extruder number, indexed from 1. Default value is 0, meaning modify all extruders.
- V: number of steps (decimal)
- S: save mileage immediately (default: true)

Example command:
`R744 E2 V5883928405943249`

### R745

Reset homed status.

This command informs the printer that the toolhead may have been moved by some source other than the stepper driver,
and so the printer should home again before it can be sure of its absolute position.

### R750

Hard Reset.
This command immediately resets the printer, and can be used to recover from a killed state (such as M112).

### R751; R752

Emergency-priority parsing.
Immediately stops extrusion.
Decelerates to a smooth stop.

- R751: finishes the currently-executing gcode, and then stops.
- R752: decelerates to a smooth stop at the next buffered move.
  Clears command buffer and movement planning buffer.

After the pause completes (i.e. when the print head stops moving), the following report is issued:

Report format [pause]

- N: line number for first command cleared from command queue (e.g. N154). [Requires line numbers to be used.] If the queue was empty or no line numbers were used, this is not reported.
- G: last movement command that finished. (e.g. G152). [Requires line numbers.]. If there were no movement commands with line numbers recorded on the queue, this is not reported.
- P: exact position of where the printhead was when the pause command was received.
- C: exact position at the end of the pause.
- deceleration: bool. Did the pause require a deceleration move?
- cropped: bool. (intended for debugging.) A deceleration move would have been planned, but it was small enough that it fell below the minimum movement threshold.
- distance: (omitted if no deceleration occurred): how long (in mm) was the deceleration move?
- sd: (omitted if SD card is not enabled): if true, this means that an sd card print was in progress and this command paused it (like M24).
- _Notable omission:_ the position at the end of the last gcode executed (G) is not reported. (However, if “deceleration" is false, the position would be exactly the reported C position.).

Example command: `R751`

**Example report [pause]**

```
pause:{"N":3,"G":2,"deceleration":true,"cropped":false,"distance":13.93,"sd":false,"P":{"X":100.324,"Y":100.000,"Z":2.000,"E":0.000,"T":0},"C":{"X":114.925,"Y":100.000,"Z":2.000,"E":0.000,"T":0}}
```

### R753

Hard Reset to Bootloader.
This command immediately jumps to the bootloader.