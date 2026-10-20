
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    uint8_t data[8];

    int fd = open("telemetry.bin", O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    ssize_t n = read(fd, data, sizeof(data));

    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    // BUG 1: Is this the correct frame size?
    if (n != 8) {
        fprintf(stderr, "Invalid frame length\n");
        return 1;
    }

    if (data[0] != 0x4E || data[1] != 0x52) {
        fprintf(stderr, "Invalid identifier\n");
        return 1;
    }

    // BUG 2: Is the version at this position?
    unsigned int version = data[2];

    unsigned int flags = data[3];

    // BUG 3: Check the byte order.
    unsigned int voltage_mv =
        ((unsigned int)data[4] | (unsigned int)data[5] << 8);

    printf("Version: %u\n", version);
    printf("Flags: %u\n", flags);
    printf("Voltage: %u mV\n", voltage_mv);

    return 0;
}
