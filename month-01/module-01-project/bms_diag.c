
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
if (argc != 2) {
        fprintf(stderr, "Usage: %s <frame.bin>\n", argv[0]);
        return 2;
    }

    uint8_t data[8];

    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }


    size_t frame_count = 0;

    while (1) {
        size_t filled = 0;

        /* Collect one complete 8-byte frame */
        while (filled < sizeof(data)) {
            ssize_t n = read(
                fd,
                data + filled,
                sizeof(data) - filled
            );

            if (n == -1) {
                if (errno == EINTR) {
                    continue;
                }

                perror("read");
                close(fd);
                return 1;
            }

            if (n == 0) {
                break;
            }

            filled += (size_t)n;
        }

        /* EOF between frames: normal end */
        if (filled == 0) {
            break;
        }

        /* EOF inside a frame: error */
        if (filled != sizeof(data)) {
            fprintf(stderr, "Invalid frame length\n");
            close(fd);
            return 1;
        }

        if (data[0] != 0x4E || data[1] != 0x52) {
            fprintf(stderr, "Invalid identifier\n");
            close(fd);
            return 1;
        }

        unsigned int version = data[2];

        if (version != 1) {
            fprintf(stderr, "Unsupported version\n");
            close(fd);
            return 1;
        }

        unsigned int flags = data[3];

        unsigned int voltage_mv =
            (unsigned int)data[4] |
            ((unsigned int)data[5] << 8);

        frame_count++;

        printf("Frame %zu\n", frame_count);
        printf("Version: %u\n", version);
        printf("Flags: %u\n", flags);
        printf("Voltage: %u mV\n", voltage_mv);
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    if (frame_count == 0) {
        fprintf(stderr, "No telemetry frames found\n");
        return 1;
    }

    printf("Frames processed: %zu\n", frame_count);

    return 0;
}
