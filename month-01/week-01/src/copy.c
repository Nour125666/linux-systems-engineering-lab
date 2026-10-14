
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char buffer[128];
    ssize_t n;
    int status = 0;

    // Step 1: Check arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s FILE\n", argv[0]);
        return 1;
    }

    // Step 2: Open the input file
    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Step 3: Read the file in blocks
    while (1) {
        n = read(fd, buffer, sizeof(buffer));

        if (n == 0) {
            break;  // End of file
        }

        if (n == -1) {
            if (errno == EINTR) {
                continue;  // Retry interrupted read
            }

            perror("read");
            status = 1;
            break;
        }

        // Step 4: Write every byte we read
        size_t offset = 0;

        while (offset < (size_t)n) {
            ssize_t written = write(
                STDOUT_FILENO,
                buffer + offset,
                (size_t)n - offset
            );

            if (written == -1) {
                if (errno == EINTR) {
                    continue;  // Retry interrupted write
                }

                perror("write");
                status = 1;
                break;
            }

            if (written == 0) {
                fprintf(stderr, "write: no progress\n");
                status = 1;
                break;
            }

            offset += (size_t)written;
        }

        if (status != 0) {
            break;
        }
    }

    // Step 5: Release the file descriptor
    if (close(fd) == -1) {
        perror("close");
        status = 1;
    }

    return status;
}
