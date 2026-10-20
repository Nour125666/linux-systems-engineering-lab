
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t stop_signal = 0;

static void handle_stop(int signal_number)
{
    /* TODO 1: Store the received signal number */
    stop_signal = signal_number;
}

int main(void)
{
    struct sigaction action = {0};

    action.sa_handler = handle_stop;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    /* Handle Ctrl+C */
    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("sigaction SIGINT");
        return 1;
    }

    /* TODO 2: Also handle a termination request */
    if (sigaction(SIGTERM, &action, NULL) == -1) {
        perror("sigaction SIGTERM");
        return 1;
    }

    printf("Telemetry PID: %ld\n", (long)getpid());
    puts("Press Ctrl+C or send SIGTERM to stop.");
    fflush(stdout);

    while (stop_signal == 0) {
        puts("Telemetry service: alive");
        sleep(1);
    }

    printf("Signal %d received. Cleaning up...\n",
           (int)stop_signal);

    puts("Telemetry service stopped safely.");

    return 0;
}
