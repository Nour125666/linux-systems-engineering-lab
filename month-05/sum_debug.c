
#include <stdio.h>

static int sum_up_to(int n)
{
    int total = 0;

    for (int i = 1; i < n; i++) {
        total += i;
    }

    return total;
}

int main(void)
{
    int result = sum_up_to(5);

    printf("Result: %d\n", result);
    return 0;
}
