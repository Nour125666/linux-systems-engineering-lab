#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t child_pid = fork();

    if (child_pid < 0) {
 perror("fork");       // TODO 1: Report fork error
        return 1;
    }

    if (child_pid == 0) {
      pid_t PID = getpid();
printf("Child's PID is %ld",(long) PID);
fflush(stdout);
  // TODO 2: Print child's actual PID
       // TODO 3: Ensure output is flushed
        _exit(42);
    }

    // Parent only from this point onward

    int status;
pid_t result = waitpid(child_pid, &status, 0);
if (result == -1)
{perror("Wait PID");
return 1;}
if (result != child_pid) {
    fprintf(stderr, "Unexpected child PID\n");
    return 1;
}
if (WIFEXITED(status)) {
    printf("Exit code: %d\n", WEXITSTATUS(status));
}

    // TODO 4: Wait for the specific child
    // TODO 5: Check waitpid() return value
    // TODO 6: Check WIFEXITED(status)
    // TODO 7: Print WEXITSTATUS(status)

    return 0;
}
