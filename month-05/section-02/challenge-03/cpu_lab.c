
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void fatal(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int fds[2];

    if (pipe(fds) == -1)
        fatal("pipe");

    int flags = fcntl(fds[0], F_GETFL);

    if (flags == -1)
        fatal("fcntl get");

    if (fcntl(fds[0], F_SETFL, flags | O_NONBLOCK) == -1)
        fatal("fcntl set");

    fprintf(stderr, "BMS monitor running, PID=%ld\n",
            (long)getpid());

    unsigned long attempts = 0;

    for (;;) {
        struct pollfd pfd = {
            .fd = fds[0],
            .events = POLLIN,
            .revents = 0
        };

        int ready = poll(&pfd, 1, 1000);

        if (ready == -1) {
            if (errno == EINTR)
                continue;
            fatal("poll");
        }

        if (ready == 0) {
            ++attempts;
            continue;
        }

        unsigned char frame[64];
        ssize_t n = read(fds[0], frame, sizeof(frame));

        if (n == -1 && errno == EAGAIN)
            continue;

        if (n == -1)
            fatal("read");

        if (n == 0)
            break;

        fprintf(stderr, "Received %zd bytes\n", n);
    }

    close(fds[0]);
    close(fds[1]);
    return EXIT_SUCCESS;
}
