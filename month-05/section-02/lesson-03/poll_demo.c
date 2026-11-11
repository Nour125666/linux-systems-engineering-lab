
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void die(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int fds[2];

    if (pipe(fds) == -1)
        die("pipe");

    // Enable nonblocking reads.
    int flags = fcntl(fds[0], F_GETFL);

    if (flags == -1)
        die("F_GETFL");

    if (fcntl(fds[0], F_SETFL,
              flags | O_NONBLOCK) == -1)
        die("F_SETFL");

    char buffer[64];

    // TEST 1: No data is available.
    ssize_t n = read(fds[0], buffer, sizeof(buffer));

    if (n == -1 && errno == EAGAIN)
        puts("TEST 1: EAGAIN - no data available");
    else {
        fprintf(stderr, "Unexpected initial read\n");
        return EXIT_FAILURE;
    }

    // TEST 2: Wait with a 500 ms timeout.
    struct pollfd pfd = {
        .fd = fds[0],
        .events = POLLIN,
        .revents = 0
    };

    int result = poll(&pfd, 1, 500);

    if (result == -1)
        die("poll");

    if (result != 0) {
        fprintf(stderr, "Expected a timeout\n");
        return EXIT_FAILURE;
    }

    puts("TEST 2: poll timed out");

    // TEST 3: Simulate receiving telemetry.
    const char message[] = "CELL1=3290mV\n";

    ssize_t sent = write(fds[1], message,
                         sizeof(message) - 1);

    if (sent != (ssize_t)(sizeof(message) - 1))
        die("write");

    result = poll(&pfd, 1, 1000);

    if (result == -1)
        die("poll");

    if (result != 1 || !(pfd.revents & POLLIN)) {
        fprintf(stderr, "Expected POLLIN\n");
        return EXIT_FAILURE;
    }

    n = read(fds[0], buffer, sizeof(buffer));

    if (n <= 0) {
        fprintf(stderr, "Expected telemetry data\n");
        return EXIT_FAILURE;
    }

    printf("TEST 3: Received %zd bytes: %.*s",
           n, (int)n, buffer);

    // TEST 4: Writer disconnects.
    if (close(fds[1]) == -1)
        die("close writer");

    result = poll(&pfd, 1, 1000);

    if (result == -1)
        die("poll");

    if (result != 1 || !(pfd.revents & POLLHUP)) {
        fprintf(stderr, "Expected POLLHUP\n");
        return EXIT_FAILURE;
    }

    n = read(fds[0], buffer, sizeof(buffer));

    if (n != 0) {
        fprintf(stderr, "Expected EOF\n");
        return EXIT_FAILURE;
    }

    puts("TEST 4: POLLHUP and EOF");

    if (close(fds[0]) == -1)
        die("close reader");

    return EXIT_SUCCESS;
}
