
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAME_SIZE 11U

static int parse_frame(const uint8_t *frame,
                       size_t frame_length,
                       uint16_t **cells_out,
                       size_t *count_out)
{
    if (frame_length < 5U ||
        frame[0] != 0xA5 ||
        frame[1] != 0x5A)
        return -1;

    size_t cell_count = frame[2];

    if (cell_count == 0U ||
        cell_count > 16U ||
        frame_length != 3U + 2U * cell_count + 2U)
        return -1;

    uint16_t *cells =
        calloc(cell_count, sizeof(*cells));

    if (cells == NULL)
        return -1;

    fprintf(stderr,
            "PARSER: header reports %zu cell values\n",
            cell_count);

    for (size_t i = 0; i <= cell_count; ++i) {
        size_t offset = 3U + 2U * i;

        cells[i] = (uint16_t)frame[offset] |
                   (uint16_t)(
                       (uint16_t)frame[offset + 1U] << 8
                   );
    }

    *cells_out = cells;
    *count_out = cell_count;

    return 0;
}

int main(void)
{
    const uint8_t frame[FRAME_SIZE] = {
        0xA5, 0x5A, 0x03,
        0xE4, 0x0C,
        0xD8, 0x0C,
        0xEE, 0x0C,
        0x7E, 0x0D
    };

    uint16_t *cells = NULL;
    size_t cell_count = 0U;

    fprintf(stderr,
            "GATEWAY: processing %zu-byte frame\n",
            sizeof(frame));

    if (parse_frame(frame, sizeof(frame),
                    &cells, &cell_count) == -1) {
        fprintf(stderr, "ERROR: frame rejected\n");
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < cell_count; ++i) {
        printf("Cell %zu: %u mV\n",
               i + 1U, (unsigned)cells[i]);
    }

    free(cells);

    return EXIT_SUCCESS;
}
