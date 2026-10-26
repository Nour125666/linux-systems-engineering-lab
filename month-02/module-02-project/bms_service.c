#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

#include "frame_parser.h"

#define UART_PATH "/tmp/ttyBMS1"
#define SOCKET_PATH "/tmp/bms.sock"

static uint16_t decode_voltage_mv(
    const uint8_t frame[FRAME_SIZE])
{
    return (uint16_t)frame[4] |
           ((uint16_t)frame[5] << 8);
}

int main(void)
{
    int uart_fd = open(
        UART_PATH,
        O_RDWR | O_NOCTTY | O_NONBLOCK
    );

    if (uart_fd == -1) {
        perror("open UART");
        return EXIT_FAILURE;
    }

    struct termios tty;

    if (tcgetattr(uart_fd, &tty) == -1) {
        perror("tcgetattr");
        close(uart_fd);
        return EXIT_FAILURE;
    }

    cfmakeraw(&tty);

    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    if (tcsetattr(uart_fd, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        close(uart_fd);
        return EXIT_FAILURE;
    }

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        close(uart_fd);
        return EXIT_FAILURE;
    }

    struct sockaddr_un address = {0};

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);

    unlink(SOCKET_PATH);

    if (bind(server_fd,
             (struct sockaddr *)&address,
             sizeof(address)) == -1) {
        perror("bind");
        close(server_fd);
        close(uart_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        unlink(SOCKET_PATH);
        close(server_fd);
        close(uart_fd);
        return EXIT_FAILURE;
    }

    printf("BMS service started\n");
    printf("UART: %s\n", UART_PATH);
    printf("IPC: %s\n", SOCKET_PATH);
    fflush(stdout);

    uint8_t buffer[64];
    uint8_t frame[FRAME_SIZE] = {0};

    size_t received = 0;
    ParserState state = WAIT_MAGIC_0;

    uint16_t latest_voltage_mv = 0;
    bool have_voltage = false;

    struct pollfd fds[2] = {
        {.fd = uart_fd, .events = POLLIN},
        {.fd = server_fd, .events = POLLIN}
    };

    int exit_status = EXIT_SUCCESS;

    while (1) {
        int ready = poll(fds, 2, 1000);

        if (ready == -1) {
            if (errno == EINTR) {
                continue;
            }

            perror("poll");
            exit_status = EXIT_FAILURE;
            break;
        }

        if (ready == 0) {
            continue;
        }

        if (fds[0].revents &
            (POLLERR | POLLHUP | POLLNVAL)) {
            fprintf(stderr, "UART disconnected or failed\n");
            exit_status = EXIT_FAILURE;
            break;
        }

        if (fds[1].revents &
            (POLLERR | POLLHUP | POLLNVAL)) {
            fprintf(stderr, "IPC listening socket failed\n");
            exit_status = EXIT_FAILURE;
            break;
        }

        if (fds[0].revents & POLLIN) {
            ssize_t n = read(
                uart_fd,
                buffer,
                sizeof(buffer)
            );

            if (n > 0) {
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

                    latest_voltage_mv =
                        decode_voltage_mv(frame);

                    have_voltage = true;

                    printf("BMS voltage updated: %.3f V\n",
                           latest_voltage_mv / 1000.0);
                    fflush(stdout);
                }
            } else if (n == 0) {
                fprintf(stderr, "UART reached EOF\n");
                exit_status = EXIT_FAILURE;
                break;
            } else if (errno != EINTR &&
                       errno != EAGAIN &&
                       errno != EWOULDBLOCK) {
                perror("UART read");
                exit_status = EXIT_FAILURE;
                break;
            }
        }

        if (fds[1].revents & POLLIN) {
            int client_fd = accept(
                server_fd,
                NULL,
                NULL
            );

            if (client_fd == -1) {
                if (errno == EINTR) {
                    continue;
                }

                perror("accept");
                exit_status = EXIT_FAILURE;
                break;
            }

            char message[128];

            if (have_voltage) {
                snprintf(
                    message,
                    sizeof(message),
                    "BMS voltage: %.3f V\n",
                    latest_voltage_mv / 1000.0
                );
            } else {
                snprintf(
                    message,
                    sizeof(message),
                    "No valid BMS data yet\n"
                );
            }

            ssize_t sent = send(
                client_fd,
                message,
                strlen(message),
                MSG_NOSIGNAL
            );

            if (sent == -1) {
                perror("send");
            }

            close(client_fd);
        }
    }

    close(server_fd);
    close(uart_fd);
    unlink(SOCKET_PATH);

    return exit_status;
}
