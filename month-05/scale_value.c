#include <stdio.h>

static int scale_value(int value)
{
    int result = value * 2;
    return result;
}

static int process_value(int input)
{
    return scale_value(input);
}

int main(void)
{
    int value = process_value(21);
    printf("%d\n", value);
    return 0;
}