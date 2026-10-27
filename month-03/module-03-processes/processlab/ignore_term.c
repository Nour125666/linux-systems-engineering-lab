#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Child PID=%ld: ignoring SIGTERM\n", (long)getpid());
    fflush(stdout);

    for (;;) {
        pause();
    }
}
