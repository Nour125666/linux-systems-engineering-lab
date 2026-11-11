
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t *receive_frame(void)
{
    uint8_t *frame = malloc(4);

    if (frame == NULL)
        return NULL;

    frame[0] = 0xA5;
    frame[1] = 0x01;
    frame[2] = 0xC8;
    frame[3] = 0x0D;

    return frame;
}

static unsigned decode_voltage(const uint8_t *frame)
{
    return ((unsigned)frame[1] << 8) | frame[2];
}

int main(void)
{
    uint8_t *frame = receive_frame();

    if (frame == NULL) {
        perror("receive_frame");
        return EXIT_FAILURE;
    }

    unsigned voltage = decode_voltage(frame);
free(frame);

printf("Voltage: %u mV\n", voltage);

    printf("Voltage: %u mV\n", voltage);
    return EXIT_SUCCESS;
}
