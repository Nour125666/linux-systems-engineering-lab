
# Battery Telemetry Diagnostic Tool

## Overview

A Linux userspace diagnostic tool written in C.

The application reads binary telemetry frames from a file,
validates their contents, and displays decoded measurements.

This project is part of the Linux Systems Engineering Lab.

## Telemetry Protocol

The project uses a custom educational 8-byte binary protocol.
It is not an official BMS communication standard.

| Byte | Description |
|------|-------------|
| 0 | Magic byte: 0x4E ('N') |
| 1 | Magic byte: 0x52 ('R') |
| 2 | Protocol version: 1 |
| 3 | Flags |
| 4 | Voltage low byte |
| 5 | Voltage high byte |
| 6–7 | Reserved |

Voltage is represented as an unsigned 16-bit
little-endian integer in millivolts.

## Features

- Linux system calls: open(), read(), close()
- Multiple-frame decoding
- Partial-read handling
- EINTR handling
- Identifier and version validation
- Detection of truncated frames
- Detection of empty input
- Error handling and exit statuses
- Automated Bash regression tests

## Build

Run:

    make

## Usage

Run:

    ./bms_diag capture.bin

The input file must contain one or more complete
8-byte frames.

## Testing

Run:

    make test

The regression suite covers eight test cases.

## Cleaning

Run:

    make clean

## Exit Status

| Status | Meaning |
|--------|---------|
| 0 | Successful processing |
| 1 | Invalid input or file I/O failure |
| 2 | Incorrect command-line usage |

## Development Tools

- GCC
- GNU Make
- Bash
- strace
- AddressSanitizer
- UndefinedBehaviorSanitizer

## Limitations

- The protocol is for educational purposes.
- Input is read from files, not live UART hardware.
- Reserved bytes are not currently validated.
- Already decoded frames may be printed before
  a later invalid frame is detected.
