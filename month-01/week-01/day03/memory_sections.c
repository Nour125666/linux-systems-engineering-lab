
#include <stdio.h>

int voltage_mv = 3300;
int alert_count;
const char label[] = "BMS";

int main(void)
{
    // TODO 1: Replace TODO_ALERTS with the number 2
    alert_count = 2;

    // TODO 2: Replace TODO_VOLTAGE with the correct variable
    printf("%s: %d mV, alerts=%d\n",
           label, voltage_mv, alert_count);

    return 0;
}
