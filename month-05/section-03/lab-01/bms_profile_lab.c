
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define FRAME_COUNT 60000U
#define CALIBRATION_ROUNDS 14000U

#define MIN_VOLTAGE 2800U
#define VOLTAGE_RANGE 650U

static uint32_t decode_voltage(uint32_t raw)
{
    return MIN_VOLTAGE + (raw % VOLTAGE_RANGE);
}

static uint32_t derive_signature(uint32_t millivolts)
{
    uint32_t state = millivolts ^ 0x9e3779b9U;

    for (uint32_t i = 0; i < CALIBRATION_ROUNDS; ++i) {
        state = state * 1664525U + 1013904223U;
        state ^= state >> 13;
    }

    return state;
}

static uint32_t process_sample(uint32_t raw)
{
    static uint32_t cache[VOLTAGE_RANGE];
    static bool calculated[VOLTAGE_RANGE] = {false};

    uint32_t voltage = decode_voltage(raw);

    // Map voltage 2800-3449 to index 0-649
    uint32_t index = voltage - MIN_VOLTAGE;

    // Calculate only if we haven't seen this voltage
    if (!calculated[index]) {
        cache[index] = derive_signature(voltage);
        calculated[index] = true;
    }

    // Reuse the cached result
    return voltage + (cache[index] & 0xffU);
}

int main(void)
{
    uint64_t checksum = 0;

    for (uint32_t i = 0; i < FRAME_COUNT; ++i)
        checksum += process_sample(i);

    printf("Samples: %u, checksum: %llu\n",
           FRAME_COUNT,
           (unsigned long long)checksum);

    return 0;
}
