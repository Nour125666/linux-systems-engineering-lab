#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

static const uint8_t *receive_frame(int frame_available,
                                    const uint8_t *sample)
{
    return frame_available ? sample : NULL;
}

static int decode_first_cell_mv(const uint8_t *frame)
{
    return ((int)frame[0] << 8) | frame[1];
}

static void process_frame(const uint8_t *frame)
{
    printf("First cell: %d mV\n", decode_first_cell_mv(frame));
}

int main(void)
{
    const uint8_t sample[] = {0x0C, 0xE4};
    const uint8_t *frame = receive_frame(0, sample);
    process_frame(frame);
    return 0;
}
