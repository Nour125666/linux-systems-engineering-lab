
#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int run_test(const char *command)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 125;
    }

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        perror("execl");
        _exit(127);
    }

    int status;

    while (waitpid(pid, &status, 0) == -1) {
        if (errno == EINTR)
            continue;

        perror("waitpid");
        return 125;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);

    return 125;
}


int main(int argc, char *argv[])
{

    int passed = 0;
int failed = 0;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s manifest.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t capacity = 0;

    while (getline(&line, &capacity, file) != -1) {

        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0' || line[0] == '#')
            continue;

        int result = run_test(line);
        if (result != 0) {
            failed++;
            fprintf(stderr, "Test failed: %s\n", line);    
        }
        else {
            passed++;
            printf("Test passed: %s\n", line);
        }}
       
    int read_error = ferror(file);

    free(line);

    if (fclose(file) != 0) {
        perror("fclose");
        return EXIT_FAILURE;
    }

    if (read_error) {
        fprintf(stderr, "Error reading manifest\n");
        return EXIT_FAILURE;
    }

    printf("\nSummary: %d passed, %d failed, %d total\n",
           passed, failed, passed + failed);

    if (failed == 0) {
        printf("All tests passed!\n");
        return EXIT_SUCCESS;
    }

    printf("Some tests failed.\n");
    return EXIT_FAILURE;
}


