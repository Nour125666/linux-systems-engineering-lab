#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *voltage = malloc(sizeof *voltage);

    if (voltage == NULL) {
        return 1;
    }

    *voltage = 3600;

    printf("Voltage: %d mV\n", *voltage);

    free(voltage);


    return 0;
}
