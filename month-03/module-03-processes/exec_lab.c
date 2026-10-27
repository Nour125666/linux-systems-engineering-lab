#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("fork");
        return 1;
    }

    if (child_pid == 0) {
        char *const args[] = {
            "uname",
            "-a",
            NULL
        };

        printf("Child PID=%ld: launching uname\n",
               (long)getpid());
        fflush(stdout);

        // TODO 1: Execute uname using execvp()

        // TODO 2: If execvp fails, print an error
if (execvp("uname", args) == -1){
perror("EXEC failed");}
        _exit(127);
    }

    int status;

  pid_t result = waitpid(child_pid, &status, 0);  // TODO 3: Wait for the specific child
 if (result == -1) {
    perror("waitpid");
    return 1;
}   // TODO 4: Handle waitpid errors
if (WIFEXITED(status)) {
    printf("Exit code: %d\n", WEXITSTATUS(status));
} else if (WIFSIGNALED(status)) {
    printf("Terminated by signal: %d\n", WTERMSIG(status));
}
    return 0;
}
