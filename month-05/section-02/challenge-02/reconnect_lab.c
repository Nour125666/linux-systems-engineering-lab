
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static int try_receive(void)
{
    int fd = open("/dev/null", O_RDONLY | O_CLOEXEC);

    if (fd == -1)
        return -1;

    unsigned char byte;
    ssize_t n = read(fd, &byte, 1);

    if (n == 0)
    close(fd);
        return 0;

    if (n == -1) {
        int saved_errno = errno;
        (void)close(fd);
        errno = saved_errno;
        return -1;
    }

    if (close(fd) == -1)
        return -1;

    return 1;
}

int main(void)
{
    fprintf(stderr, "INFO: receiver started, PID=%ld\n",
            (long)getpid());

    for (unsigned attempt = 1; ; ++attempt) {

        int result = try_receive();

        if (result == -1) {
            perror("ERROR: receiver operation");
            fprintf(stderr,
                    "INFO: staying alive for inspection\n");
            sleep(45);
            return EXIT_FAILURE;
        }

        if (result == 0 && attempt % 10 == 0)
            fprintf(stderr,
                    "WARN: no frame, attempt=%u\n",
                    attempt);

        struct timespec delay = {
            .tv_sec = 0,
            .tv_nsec = 250000000L
        };

        (void)nanosleep(&delay, NULL);
    }
}
