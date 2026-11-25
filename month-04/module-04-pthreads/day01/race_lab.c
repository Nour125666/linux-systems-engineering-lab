
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORKER_COUNT 2
#define ITERATIONS 100000

struct worker_status {
    int error;
};

static long counter = 0;
static pthread_mutex_t counter_mutex =
    PTHREAD_MUTEX_INITIALIZER;

static void *worker(void *arg)
{
    struct worker_status *status = arg;
    status->error = 0;

    for (int i = 0; i < ITERATIONS; i++) {

        int rc = pthread_mutex_lock(&counter_mutex);

        if (rc != 0) {
            status->error = rc;
            return NULL;
        }

        counter++;

        rc = pthread_mutex_unlock(&counter_mutex);

        if (rc != 0) {
            status->error = rc;
            return NULL;
        }
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[WORKER_COUNT];
    struct worker_status statuses[WORKER_COUNT] = {{0}};

    int created = 0;
    int failed = 0;
    int rc;

    for (int i = 0; i < WORKER_COUNT; i++) {

        rc = pthread_create(&threads[i], NULL,
                            worker, &statuses[i]);

        if (rc != 0) {
            fprintf(stderr, "pthread_create: %s\n",
                    strerror(rc));
            failed = 1;
            break;
        }

        created++;
    }

    for (int i = 0; i < created; i++) {

        rc = pthread_join(threads[i], NULL);

        if (rc != 0) {
            fprintf(stderr, "pthread_join: %s\n",
                    strerror(rc));
            failed = 1;
            continue;
        }

        if (statuses[i].error != 0) {
            fprintf(stderr, "worker %d: %s\n",
                    i, strerror(statuses[i].error));
            failed = 1;
        }
    }

    if (failed) {
        return EXIT_FAILURE;
    }

    printf("Expected counter value: %d\n",
           WORKER_COUNT * ITERATIONS);

    printf("Actual counter value:   %ld\n",
           counter);

    return EXIT_SUCCESS;
}
