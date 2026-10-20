
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    printf("Launcher PID: %ld\n", (long)getpid());
    fflush(stdout);

    pid_t child = fork();

    if (child == -1) {
        perror("fork");
        return 1;
    }

    if (child == 0) {
        /* This code runs inside the child */
        printf("Child PID: %ld\n", (long)getpid());
        fflush(stdout);

        /* BUG 1: The program name is wrong */
        execlp("echo", "echo",
               "Hello from the child program",
               (char *)NULL);
        _exit(127);
    }

    /* This code runs inside the parent */
    int status;

    /* BUG 2: Are we waiting for the correct PID? */
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status)) {
        printf("Child exit code: %d\n",
               WEXITSTATUS(status));
    }

    return 0;
}
