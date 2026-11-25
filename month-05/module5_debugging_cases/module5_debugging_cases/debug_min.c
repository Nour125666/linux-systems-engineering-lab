#include <stdio.h>
#include <stddef.h>

static int find_min_cell_mv(const int cells_mv[], size_t count)
{
    int minimum_mv = 0;

    for (size_t i = 0; i < count; i++) {
        if (cells_mv[i] < minimum_mv) {
            minimum_mv = cells_mv[i];
        }
    }
    return minimum_mv;
}

int main(void)
{
    const int cells_mv[] = {3300, 3280, 3310, 3250};
    const size_t count = sizeof cells_mv / sizeof cells_mv[0];
    printf("Minimum cell voltage: %d mV\n", find_min_cell_mv(cells_mv, count));
    return 0;
}
