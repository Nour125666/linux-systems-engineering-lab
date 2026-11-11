
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void fatal(const char *what)
{
    perror(what);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int channel[2];

    if (pipe(channel) == -1)
        fatal("pipe");

    pid_t producer = fork();

    if (producer == -1)
        fatal("fork");

    if (producer == 0) {
        if (close(channel[0]) == -1)
            _exit(2);

        const char frame[] = "CELL1=3290mV\n";

        if (write(channel[1], frame,
                  sizeof(frame) - 1) !=
            (ssize_t)(sizeof(frame) - 1))
            _exit(3);

        if (close(channel[1]) == -1)
            _exit(4);

        _exit(0);
    }

    fprintf(stderr,
            "[receiver] PID=%ld, waiting for telemetry\n",
            (long)getpid());

    char buffer[64];

    for (;;) {
        ssize_t n = read(channel[0],
                         buffer, sizeof(buffer));

        if (n == 0)
            break;

        if (n == -1) {
            if (errno == EINTR)
                continue;

            fatal("read");
        }

        fprintf(stderr,
                "[receiver] got %zd bytes: %.*s",
                n, (int)n, buffer);
    }

    if (close(channel[0]) == -1)
        fatal("close");

    int status;

    if (waitpid(producer, &status, 0) == -1)
        fatal("waitpid");

    fprintf(stderr,
            "[receiver] EOF reached, shutdown complete\n");

    return EXIT_SUCCESS;
}
