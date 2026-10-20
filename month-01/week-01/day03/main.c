#include <stdio.h>
#include "sensor.h"

int main(void)
{
    int voltage = get_voltage_mv();

    printf("Sensor voltage: %d mV\n", voltage);

    return 0;
}
