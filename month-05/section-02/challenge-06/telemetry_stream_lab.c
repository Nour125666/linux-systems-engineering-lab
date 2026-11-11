
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define FRAME_SIZE 8U

static void die(const char *where)
{
    perror(where);
    exit(EXIT_FAILURE);
}

static void send_bytes(int fd, const uint8_t *data,
                       size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = write(fd, data + sent,
                          length - sent);

        if (n == -1 && errno == EINTR)
            continue;

        if (n <= 0)
            die("write");

        sent += (size_t)n;
    }
}

static void delay_sender(void)
{
    struct timespec req = {
        .tv_sec = 0,
        .tv_nsec = 600000000L
    };

    while (nanosleep(&req, &req) == -1) {
        if (errno != EINTR)
            die("nanosleep");
    }
}

static void sender(int fd)
{
    const uint8_t frame[FRAME_SIZE] = {
        0xA5, 0x5A, 0x08, 0x01,
        0x0C, 0xDA, 0x0D, 0x7E
    };

    send_bytes(fd, frame, 3);

    fprintf(stderr,
            "SENDER: transmitted first segment\n");

    delay_sender();

    send_bytes(fd, frame + 3,
               FRAME_SIZE - 3);

    fprintf(stderr,
            "SENDER: transmitted remaining segment\n");

    if (close(fd) == -1)
        die("sender close");
}

static ssize_t receive_frame(int fd,
                             uint8_t *frame,
                             size_t capacity)
{
    size_t received = 0;

    while (received < capacity) {
        ssize_t n = read(fd,
                         frame + received,
                         capacity - received);

        if (n > 0) {
            received += (size_t)n;
            continue;
        }

        if (n == 0)
            return (ssize_t)received;  // EOF before full frame

        if (errno == EINTR)
            continue;

        return -1;
    }

    return (ssize_t)received;
}

int main(void)
{
    int pair[2];

    if (socketpair(AF_UNIX, SOCK_STREAM,
                   0, pair) == -1)
        die("socketpair");

    pid_t child = fork();

    if (child == -1)
        die("fork");

    if (child == 0) {
        if (close(pair[0]) == -1)
            die("child close");

        sender(pair[1]);
        _exit(EXIT_SUCCESS);
    }

    if (close(pair[1]) == -1)
        die("parent close");

    fprintf(stderr,
            "RECEIVER: expecting %u-byte telemetry frame\n",
            (unsigned)FRAME_SIZE);

    uint8_t frame[FRAME_SIZE] = {0};

    ssize_t count = receive_frame(
        pair[0], frame, sizeof(frame)
    );

    if (count == -1)
        die("receive_frame");

    fprintf(stderr,
            "RECEIVER: received %zd byte(s)\n",
            count);

    int status = 0;

    if (waitpid(child, &status, 0) == -1)
        die("waitpid");

    if (close(pair[0]) == -1)
        die("receiver close");

    if (!WIFEXITED(status) ||
        WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr,
                "ERROR: sender did not exit successfully\n");
        return EXIT_FAILURE;
    }

    if (count != (ssize_t)FRAME_SIZE) {
        fprintf(stderr,
                "ERROR: telemetry frame rejected: "
                "expected %u bytes\n",
                (unsigned)FRAME_SIZE);
        return EXIT_FAILURE;
    }

    printf("FRAME OK:");

    for (size_t i = 0; i < FRAME_SIZE; ++i)
        printf(" %02X", frame[i]);

    putchar('\n');

    return EXIT_SUCCESS;
}