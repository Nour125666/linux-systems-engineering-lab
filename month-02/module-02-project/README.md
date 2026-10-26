# Module 2 — Linux BMS Telemetry Service

## Overview

A Linux systems programming project that receives simulated
BMS measurements over a virtual UART, validates binary frames,
and exposes the latest valid voltage through a Unix-domain socket.

## Architecture

Python BMS Simulator
        |
        v
socat Virtual UART (/tmp/ttyBMS0)
        |
        v
Linux BMS Service (/tmp/ttyBMS1)
        |
        +-- termios configuration
        +-- nonblocking read()
        +-- poll() event loop
        +-- binary frame parser
        +-- CRC-8 validation
        +-- latest valid voltage
        |
        v
Unix Socket (/tmp/bms.sock)
        |
        v
C Monitoring Client

## Protocol

Frame length: 8 bytes

| Byte | Description |
|------|-------------|
| 0 | Magic 0x4E |
| 1 | Magic 0x52 |
| 2 | Protocol version 0x01 |
| 3 | Flags |
| 4 | Voltage low byte |
| 5 | Voltage high byte |
| 6 | Reserved |
| 7 | CRC-8 |

CRC-8 polynomial: 0x07
Initial value: 0x00
CRC covers bytes 0 through 6.

Voltage is encoded as an unsigned 16-bit
little-endian integer in millivolts.

## Requirements

- Linux
- GCC
- Python 3
- socat

## Build

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g \
    bms_service.c frame_parser.c crc8.c -o bms_service

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g \
    ipc_client.c -o ipc_client

## Run

Terminal 1:

    socat -d -d PTY,raw,echo=0,link=/tmp/ttyBMS0 \
        PTY,raw,echo=0,link=/tmp/ttyBMS1

Terminal 2:

    ./bms_service

Terminal 3:

    ./ipc_client

The client initially reports:

    Received: No valid BMS data yet

After a valid UART frame is received, the client reports
the latest validated voltage.

## Automated Tests

CRC test:

    gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g \
        test_crc8.c crc8.c -o test_crc8
    ./test_crc8

Parser tests:

    gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g \
        test_frame_parser.c frame_parser.c crc8.c \
        -o test_frame_parser
    ./test_frame_parser

Verified tests:

- CRC-8 reference vector
- Valid frame
- Garbage before frame
- Incomplete frame
- Repeated magic bytes
- Lost-byte recovery
- Valid voltage propagation through IPC
- Corrupted frame rejection
- Latest measurement preservation
- Sequential client connections

## Known Limitations

- The service stores only the latest valid voltage,
  not a complete measurement history.
- The current server makes a single send() attempt;
  partial socket writes are not fully handled.
- Graceful SIGINT/SIGTERM cleanup is not implemented.
- No reconnect mechanism is implemented for UART failure.
- The client response uses a simple text format.
- This is a laboratory demonstration, not a
  production-ready safety-critical BMS service.

## Skills Demonstrated

- POSIX file descriptors
- termios serial configuration
- Nonblocking I/O
- poll() event multiplexing
- Binary protocol parsing
- CRC-8 validation
- Unix-domain sockets
- Interprocess communication
- C unit testing
- Linux debugging
