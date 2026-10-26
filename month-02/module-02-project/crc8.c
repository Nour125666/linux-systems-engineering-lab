#include "crc8.h"

uint8_t crc8_calculate(const uint8_t *data, size_t length)
{
    uint8_t crc = 0x00;

    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (unsigned bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x07);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }

    return crc;
}
