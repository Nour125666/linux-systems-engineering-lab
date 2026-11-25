
#include <stdio.h>

static void update_temperature(int *sensor_ptr)
{
    *sensor_ptr += 5;
}

int main(void)
{
    int temperature = 25;
    int *ptr = &temperature;

    update_temperature(ptr);

    printf("Temperature: %d\n", temperature);
    return 0;
}
