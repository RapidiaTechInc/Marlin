# Rapidia 3D Printer Firmware

Marlin is an open source project which drives many of the world's 3D printers. It is considered to be a reliable and professional-grade firwmare project. Additional documentation can be found at the [Marlin Home Page](http://marlinfw.org/).

Rapidia has made some modifications to Marlin to work better with our hardware and host software. Because this project is licensed under [version 3.0 of the GNU General Public License](./LICENSE), the source code and license must be made available with every distribution of the Marlin firmware.

Most Rapidia-specific changes to the code can be identified by the surrounding RAPIDIA\_\* macros. A list of the new g-code commands created for and existing commands modified for Rapidia use has been provided below.

## Build with Docker

- [Install Docker](https://docs.docker.com/get-docker/)
- Run `docker build . --tag arduino-cli`
- Run `docker run --rm -v ${PWD}:/Marlin -w /Marlin arduino-cli:latest /bin/sh ./arduino-build.sh`

## Changes

### M155 [S(u8:seconds)][h(s32:milliseconds)] [P,C,R,X,E(0,1)]

Auto-reporting. In the original firmware, only the S option is available. S sets the interval at which temperature auto-reporting occurs. H sets the interval at which the heartbeat status update occurs. Temperature and heartbeat reports occur separately, but they are both enabled by this command. P,C,R, etc. can enable/disable individual status updates in that heartbeat. Some of these options are disabled by default (\*). The report is issued as a json object and can contain the following entries:

- P: current plan position. (After executing all the plans in the buffer, - the toolhead will be here.)
- C: actual current position. (May not be very useful for logic, but could be nifty for UI reasons.)
- F: feedrate shown with plan position.
- R: per-axis relative mode flag enabled/disabled. (Reported as a string containing the axes in relative mode, e.g. “XYZ")
- X\*: dualx state
- E: Endstops states. Reported as a string: endstop state for X_MIN through Z_MIN (reported as ‘x’, ‘y’, ‘z’ in lower case), and X_MAX through Z_MAX (reported as ‘X’, ‘Y’, ‘Z’ in upper case)

Example command:
`M155 S3 H2000 P1 C0 R1 X0`

**Example report [H]**

```
H:{"P":{"X":113.925,"Y":100.000,"Z":2.000,"E":0.000,"F":33.333,"T":0},"C":{"X":113.925,"Y":100.000,"Z":2.000,"E":0.000,"T":0},"R":"","ES":"xXZ"}
```

Note that the “F" and “T" entries in the position object refer to the current feedrate and tool respectively.

### M710 T(0-3)

Set Timer. There are 4 timers which can be used in conditional gcode execution (see M711). This command resets the specified timer.

### M711 T(0-3) [A(u32)][b(u32)] [N(u8)]

Conditional Execution (on Timer). This command checks if the specified timer T has elapsed at least (A) or fewer than (B) the specified number of milliseconds. If it has not -- that is, if the condition evaluates to false -- then the next specified number N lines of gcode received will be skipped. Only gcode which starts with a ‘G’ or ‘T’ will be skipped; ‘M’ code will neither be skipped nor will it count toward the number of lines which are to be skipped.

### M730; M731

Enable/Disable movement-complete auto-reporting.
These respectively enable and disable reporting when individual lines of gcode have completed execution. (Note that this does not enable synchronous movement.)

### M735 [S(u8)][i(u16:milliseconds)]

Z_MAX (nozzle plug) signal processing.

Arguments:

- S: sample threshold.
- I: minimum sample interval.

Setting S to an integer greater than 1 makes it so that Marlin must read S positive reads on the Z_MAX endstop pin (i.e. the nozzle plug pin) in a row for it to properly count as triggered. By default, it will sample no faster than once per ms. Setting I to a value greater than 1 means that the endstop will be sampled (no faster than) every I milliseconds.

Warning: setting S0 means that Marlin requires 0 positive reads for the endstop to count as “triggered". In other words, the endstop will always be triggered. This is likely to be useful only for debugging nozzle plug detection.

### M733

Pin test.

Directly pulses various pins. This will cause the firmware to forget which pins are currently HIGH and LOW, so only use this for testing purposes, never in the context of an actual print job.

### M736; M737

Lamp on/Lamp off
For now, these commands are aliases of M106 and M107.
M751/M752

### M751; M752

Emergency-priority parsing.
Immediately stops extrusion.
Decelerates to a smooth stop.

- M751: finishes the currently-executing gcode, and then stops.
- M752: decelerates to a smooth stop at the next buffered move.
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

Example command: `M751`

**Example report [pause]**

```
pause:{"N":3,"G":2,"deceleration":true,"cropped":false,"distance":13.93,"sd":false,"P":{"X":100.324,"Y":100.000,"Z":2.000,"E":0.000,"T":0},"C":{"X":114.925,"Y":100.000,"Z":2.000,"E":0.000,"T":0}}
```
