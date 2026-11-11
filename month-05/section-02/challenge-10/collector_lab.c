
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void fail(const char *what)
{
    perror(what);
    exit(EXIT_FAILURE);
}

static void send_all(int fd, const char *text)
{
    size_t length = strlen(text);
    size_t offset = 0;

    while (offset < length) {
        ssize_t n = write(fd, text + offset,
                          length - offset);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0)
            fail("worker write");

        offset += (size_t)n;
    }
}

int main(void)
{
    int channel[2];

    if (pipe(channel) == -1)
        fail("pipe");

    pid_t diagnostic_pid = fork();

    if (diagnostic_pid == -1)
        fail("fork diagnostics");

    if (diagnostic_pid == 0) {
        if (close(channel[0]) == -1)
            _exit(127);
         if (close(channel[1]) == -1)
        _exit(127);
        execlp("sleep", "sleep", "7", (char *)NULL);

        perror("exec diagnostics");
        _exit(127);
    }

    pid_t worker_pid = fork();

    if (worker_pid == -1)
        fail("fork worker");

    if (worker_pid == 0) {
        if (close(channel[0]) == -1)
            _exit(127);

        send_all(channel[1], "BMS CELL01=3300mV\n");

        if (close(channel[1]) == -1)
            _exit(127);

        _exit(EXIT_SUCCESS);
    }

    if (close(channel[1]) == -1)
        fail("parent close write");

    fprintf(stderr,
            "COLLECTOR pid=%ld, WORKER pid=%ld, "
            "DIAGNOSTIC pid=%ld\n",
            (long)getpid(),
            (long)worker_pid,
            (long)diagnostic_pid);

    int worker_status;

    if (waitpid(worker_pid, &worker_status, 0) == -1)
        fail("waitpid worker");

    fprintf(stderr,
            "COLLECTOR worker finished; "
            "receiving until EOF\n");

    char buffer[64];

    for (;;) {
        ssize_t n = read(channel[0], buffer,
                         sizeof(buffer));

        if (n < 0 && errno == EINTR)
            continue;

        if (n < 0)
            fail("collector read");

        if (n == 0)
            break;

        if (fwrite(buffer, 1, (size_t)n, stdout)
            != (size_t)n)
            fail("fwrite");

        if (fflush(stdout) == EOF)
            fail("fflush");

        fprintf(stderr,
                "COLLECTOR received %zd bytes; "
                "awaiting stream completion\n", n);
    }

    fprintf(stderr, "COLLECTOR EOF reached\n");

    if (close(channel[0]) == -1)
        fail("collector close read");

    if (kill(diagnostic_pid, SIGTERM) == -1 &&
        errno != ESRCH)
        fail("stop diagnostics");

    int diagnostics_status;

    if (waitpid(diagnostic_pid,
                &diagnostics_status, 0) == -1)
        fail("waitpid diagnostics");

    fprintf(stderr, "COLLECTOR shutdown complete\n");

    return EXIT_SUCCESS;
}
