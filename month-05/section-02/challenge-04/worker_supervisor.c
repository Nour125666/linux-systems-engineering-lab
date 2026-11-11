
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define WORKER_COUNT 3

static void run_worker(unsigned id)
{
    fprintf(stderr,
            "Worker %u: diagnostic complete, PID=%ld\n",
            id, (long)getpid());

    _exit(EXIT_SUCCESS);
}

int main(void)
{
    fprintf(stderr,
            "Supervisor started, PID=%ld\n",
            (long)getpid());

    for (unsigned i = 0; i < WORKER_COUNT; ++i) {
        pid_t child = fork();

        if (child == -1) {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (child == 0)
            run_worker(i + 1);

        fprintf(stderr,
                "Supervisor: launched worker PID=%ld\n",
                (long)child);
    }

    fprintf(stderr,
            "Supervisor: monitoring workers...\n");

    sleep(300);

    return EXIT_SUCCESS;
}
