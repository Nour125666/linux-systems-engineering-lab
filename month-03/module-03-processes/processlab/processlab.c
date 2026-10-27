#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define GRACE_SECONDS 5

static volatile sig_atomic_t stop_requested = 0;

static void handle_signal(int signo)
{
    (void)signo;
    stop_requested = 1;
}

static int install_handlers(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        return -1;
    }
    return 0;
}

static int elapsed_at_least(const struct timespec *start, int seconds)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
        return -1;
    }
    time_t whole = now.tv_sec - start->tv_sec;
    return whole > seconds ||
           (whole == seconds && now.tv_nsec >= start->tv_nsec);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s PROGRAM [ARGS...]\n", argv[0]);
        return 2;
    }

    /* Install before fork to avoid an unprotected startup window. */
    if (install_handlers() == -1) {
        return 1;
    }

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return 1;
    }
if (child_pid == 0) {
        /* A launched program must not inherit our custom handlers. */
        struct sigaction sa = {0};
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGINT, &sa, NULL) == -1 ||
            sigaction(SIGTERM, &sa, NULL) == -1) {
            perror("child sigaction");
            _exit(127);
        }
        execvp(argv[1], &argv[1]);
        perror("execvp");
        _exit(127);
    }

    printf("[ProcessLab] Started child PID=%ld\n", (long)child_pid);
    fflush(stdout);

    int status = 0;
    int term_sent = 0;
    int kill_sent = 0;
    struct timespec term_time = {0};

    for (;;) {
        pid_t result = waitpid(child_pid, &status, WNOHANG);
        if (result == child_pid) {
            if (WIFEXITED(status)) {
                printf("[ProcessLab] Child exited with code %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("[ProcessLab] Child terminated by signal %d\n", WTERMSIG(status));
            }
            break;
        }
        if (result == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("waitpid");
            return 1;
        }

        if (stop_requested && !term_sent) {
            puts("[ProcessLab] Shutdown requested");
            if (clock_gettime(CLOCK_MONOTONIC, &term_time) == -1) {
                perror("clock_gettime");
                /* Best effort: do not leave the child running on clock failure. */
                if (kill(child_pid, SIGKILL) == -1 && errno != ESRCH)
                    perror("kill SIGKILL");
                while (waitpid(child_pid, &status, 0) == -1 && errno == EINTR) {}
                return 1;
            }
            if (kill(child_pid, SIGTERM) == -1) {
                if (errno != ESRCH) perror("kill SIGTERM");
            } else {
                printf("[ProcessLab] SIGTERM sent to child %ld\n", (long)child_pid);
            }
            term_sent = 1;
        }

        if (term_sent && !kill_sent) {
            int expired = elapsed_at_least(&term_time, GRACE_SECONDS);
            if (expired == -1) {
                perror("clock_gettime");
                expired = 1; /* Fail safe: escalate rather than wait forever. */
            }
            if (expired) {
                puts("[ProcessLab] Grace period expired; escalating to SIGKILL");
                if (kill(child_pid, SIGKILL) == -1) {
                    if (errno != ESRCH) perror("kill SIGKILL");
                } else {
                    printf("[ProcessLab] SIGKILL sent to child %ld\n", (long)child_pid);
                }
                kill_sent = 1;
            }
        }

        /* A short polling interval makes shutdown more responsive. */
        struct timespec pause_time = {.tv_sec = 0, .tv_nsec = 200000000L};
        (void)nanosleep(&pause_time, NULL);
    }
    return 0;
}
