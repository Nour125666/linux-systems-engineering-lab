
#define _DEFAULT_SOURCE
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "frame_parser.h"

/* Check the protocol header and version. */
static bool validate_frame(const uint8_t frame[FRAME_SIZE])
{
    if (frame[0] != 0x4E) {
        fprintf(stderr, "Invalid first magic byte\n");
        return false;
    }

    if (frame[1] != 0x52) {
        fprintf(stderr, "Invalid second magic byte\n");
        return false;
    }

    if (frame[2] != 0x01) {
        fprintf(stderr, "Invalid protocol version\n");
        return false;
    }

    return true;
}

/* Decode a 16-bit little-endian voltage in millivolts. */
static uint16_t decode_voltage_mv(const uint8_t frame[FRAME_SIZE])
{
    return (uint16_t)frame[4] |
           ((uint16_t)frame[5] << 8);
}

int main(void)
{
    const char *device = "/tmp/ttyBMS1";

    int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Configure the serial device. */
    struct termios tty;

    if (tcgetattr(fd, &tty) == -1) {
        perror("tcgetattr");
        close(fd);
        return EXIT_FAILURE;
    }

    cfmakeraw(&tty);

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        close(fd);
        return EXIT_FAILURE;
    }

    /* Initialize the receive buffer and parser. */
    uint8_t buffer[64];
    uint8_t frame[FRAME_SIZE] = {0};

    size_t received = 0;
    ParserState state = WAIT_MAGIC_0;
    int exit_status = EXIT_SUCCESS;

    printf("Listening on %s...\n", device);
    fflush(stdout);

    /* Continuously receive incoming data. */

struct pollfd pfd = {
    .fd = fd,
    .events = POLLIN,
    .revents = 0
};
    while (1) {
pfd.revents = 0;

int ready = poll(&pfd, 1, 1000);

if (ready == -1) {
    if (errno == EINTR) {
        continue;
    }

    perror("poll");
    exit_status = EXIT_FAILURE;
    break;
}

if (ready == 0) {
    fprintf(stderr, "BMS timeout: no data for 1 second\n");
    continue;
}

if (pfd.revents & (POLLERR | POLLNVAL)) {
    fprintf(stderr, "Serial device poll error\n");
    exit_status = EXIT_FAILURE;
    break;
}

if (pfd.revents & POLLHUP) {
    fprintf(stderr, "Serial connection hangup\n");
    exit_status = EXIT_FAILURE;
    break;
}

if (!(pfd.revents & POLLIN)) {
    continue;
}
        ssize_t n = read(fd, buffer, sizeof(buffer));

        if (n == -1) {
    if (errno == EINTR) {
        continue;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
    }

    perror("read");
    exit_status = EXIT_FAILURE;
    break;
}

        if (n == 0) {
            fprintf(stderr, "Serial connection closed\n");
            break;
        }

        /* Feed every received byte into the parser. */
        for (ssize_t i = 0; i < n; i++) {
            bool complete = parser_push_byte(
                buffer[i],
                frame,
                &received,
                &state
            );

            if (!complete) {
                continue;
            }

            /* A complete candidate frame is available. */
            if (!validate_frame(frame)) {
                fprintf(stderr, "Invalid BMS frame\n");
                continue;
            }

            uint16_t voltage_mv = decode_voltage_mv(frame);

            printf("Voltage: %u mV\n",
                   (unsigned)voltage_mv);

            printf("Voltage: %.3f V\n",
                   voltage_mv / 1000.0);

            fflush(stdout);
        }
    }

    close(fd);
    return exit_status;
}
