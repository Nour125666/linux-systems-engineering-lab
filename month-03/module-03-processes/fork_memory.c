#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int global_value = 10;

int main(void)
{
    int stack_value = 20;

    int *heap_value = malloc(sizeof(*heap_value));

    if (heap_value == NULL) {
        perror("malloc");
        return 1;
    }

    *heap_value = 30;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        free(heap_value);
        return 1;
    }

    if (pid == 0) {
        global_value = 100;
        stack_value = 200;
        *heap_value = 300;

        printf("CHILD:  global=%d stack=%d heap=%d\n",
               global_value, stack_value, *heap_value);

        free(heap_value);
        _exit(0);
    }

    printf("PARENT: global=%d stack=%d heap=%d\n",
           global_value, stack_value, *heap_value);

    free(heap_value);

    return 0;
}
