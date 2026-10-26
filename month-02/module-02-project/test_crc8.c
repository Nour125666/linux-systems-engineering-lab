#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "crc8.h"

int main(void)
{
    const uint8_t data[] = {
        '1', '2', '3', '4', '5',
        '6', '7', '8', '9'
    };

    uint8_t result = crc8_calculate(data, sizeof(data));

    printf("CRC-8: 0x%02X\n", result);

    assert(result == 0xF4);

    puts("PASS: CRC-8 reference vector");
    return 0;
}
