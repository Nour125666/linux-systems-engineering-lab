
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <device-path>\n",
                argv[0]);
        return 2;
    }

    const char *path = argv[1];

    int fd = open(path, O_RDWR | O_NOCTTY | O_CLOEXEC);

    if (fd == -1) {
        int saved_errno = errno;

        fprintf(stderr,
                "Cannot open %s: %s (errno=%d)\n",
                path,
                strerror(saved_errno),
                saved_errno);
        return 1;
    }

    printf("Successfully opened %s, fd=%d\n", path, fd);

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
