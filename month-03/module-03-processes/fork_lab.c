#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    printf("Starting process PID=%ld\n", (long)getpid());
    fflush(stdout);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("CHILD: PID=%ld, PPID=%ld\n",
               (long)getpid(),
               (long)getppid());
    } else {
        printf("PARENT: PID=%ld, child PID=%ld\n",
               (long)getpid(),
               (long)pid);
    }

    return 0;
}
