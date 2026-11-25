
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct worker_args {
    int worker_id;
};

static void *worker(void *arg)
{
    const struct worker_args *data = arg;

    printf("worker %d: process PID=%ld\n",
           data->worker_id, (long)getpid());

    return NULL;
}

int main(void)
{
    pthread_t thread;
    struct worker_args args = {.worker_id = 1};

    int rc = pthread_create(&thread, NULL, worker, &args);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(rc));
        return EXIT_FAILURE;
    }

    puts("main: worker created");

    rc = pthread_join(thread, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(rc));
        return EXIT_FAILURE;
    }

    puts("main: worker joined");
    return EXIT_SUCCESS;
}
