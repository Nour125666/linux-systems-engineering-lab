#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <time.h>

static volatile sig_atomic_t stop_requested = 0;

static void handle_signal(int signo)
{
    (void)signo;
    stop_requested = 1;
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = handle_signal;

 sigemptyset(&sa.sa_mask);   // TODO 1: Initialize sa.sa_mask.
  sa.sa_flags = 0;

if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    return 1;
}

if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("sigterm");
    return 1;
}  // TODO 2: Set sa.sa_flags to 0.
    // TODO 3: Install handler for SIGINT.
    // TODO 4: Install handler for SIGTERM.

    printf("Running. Press Ctrl+C to request shutdown.\n");

    while (!stop_requested) {
        puts("Working...");

        struct timespec delay = {
            .tv_sec = 1,
            .tv_nsec = 0
        };

      nanosleep(&delay, NULL);  // TODO 5: Sleep using nanosleep().
    }

    puts("Shutdown requested. Cleaning up...");
    return 0;
}
