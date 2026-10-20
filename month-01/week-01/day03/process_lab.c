
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("Starting process...\n");

    // BUG 1: Is this actually our process ID?
    printf("PID: %ld\n", (long)getpid());

    // BUG 2: Is this actually our parent's ID?
    printf("PPID: %ld\n", (long)getppid());

    // Make output appear immediately
    fflush(stdout);

    // Stay alive for 15 seconds
    sleep(15);

    printf("Process finished.\n");

    return 0;
}
