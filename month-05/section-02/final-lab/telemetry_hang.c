
#define _POSIX_C_SOURCE 200809L

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void fatal(const char *what)
{
    perror(what);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int fds[2];

    if (pipe(fds) == -1)
        fatal("pipe");

    fprintf(stderr,
            "INFO: telemetry receiver running, PID=%ld\n",
            (long)getpid());

    struct pollfd pfd = {
        .fd = fds[0],
        .events = POLLIN,
        .revents = 0
    };

    int ready = poll(&pfd, 1, 1000);

    if (ready == -1)
        fatal("poll");

    if (ready == 0)
        fprintf(stderr, "WARN: telemetry timeout\n");

    fprintf(stderr, "INFO: attempting to receive frame\n");

    unsigned char frame[64];

    ssize_t received = read(fds[0], frame, sizeof(frame));

    if (received == -1)
        fatal("read");

    fprintf(stderr, "INFO: received %zd bytes\n", received);

    if (close(fds[0]) == -1)
        fatal("close reader");

    if (close(fds[1]) == -1)
        fatal("close writer");

    return EXIT_SUCCESS;
}
