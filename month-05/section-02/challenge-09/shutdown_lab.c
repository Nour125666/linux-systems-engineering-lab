
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t shutdown_requested = 0;

static void fatal(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

static void handle_shutdown(int signo)
{
    (void)signo;
    shutdown_requested = 1;
}

static void install_signal_handler(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = handle_shutdown;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1)
        fatal("sigaction");
}

static void prepare_runtime(void)
{
    sigset_t signals;

    if (sigemptyset(&signals) == -1)
        fatal("sigemptyset");

    if (sigaddset(&signals, SIGTERM) == -1)
        fatal("sigaddset");

    if (sigprocmask(SIG_BLOCK, &signals, NULL) == -1)
        fatal("sigprocmask");

    fprintf(stderr,
            "INFO: preparing BMS monitoring runtime\n");
}

int main(void)
{
    install_signal_handler();
    prepare_runtime();

    fprintf(stderr,
            "INFO: BMS service ready, PID=%ld\n",
            (long)getpid());

    unsigned long heartbeat = 0;

    while (!shutdown_requested) {
        struct timespec delay = {
            .tv_sec = 0,
            .tv_nsec = 200000000L
        };

        if (nanosleep(&delay, NULL) == -1 &&
            errno != EINTR)
            fatal("nanosleep");

        ++heartbeat;

        if (heartbeat % 10UL == 0UL)
            fprintf(stderr,
                    "INFO: monitoring active, heartbeat=%lu\n",
                    heartbeat);
    }

    fprintf(stderr,
            "INFO: shutdown requested, releasing resources\n");

    fprintf(stderr,
            "INFO: BMS service stopped gracefully\n");

    return EXIT_SUCCESS;
}
