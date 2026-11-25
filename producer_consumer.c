
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Shared job state
static int job = 0;
static int ready = 0;
static int consumer_error = 0;

// Synchronization
static pthread_mutex_t mutex =
    PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t job_available =
    PTHREAD_COND_INITIALIZER;

// CONSUMER THREAD
static void *consumer(void *arg)
{
    (void)arg;

    int rc = pthread_mutex_lock(&mutex);

    if (rc != 0) {
        consumer_error = rc;
        return NULL;
    }

    // TODO 1:
    // While ready == 0, wait for a job.
while (ready == 0) {
        rc = pthread_cond_wait(&job_available, &mutex);
        if (rc != 0) {
            consumer_error = rc;
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
    }

    int received = job;

    rc = pthread_mutex_unlock(&mutex);

    if (rc != 0) {
        consumer_error = rc;
        return NULL;
    }

    printf("Consumer received job: %d\n", received);

    return NULL;
}

int main(void)
{
    pthread_t thread;

    // Create consumer
    int rc = pthread_create(&thread, NULL,
                            consumer, NULL);

    if (rc != 0) {
        fprintf(stderr, "pthread_create: %s\n",
                strerror(rc));
        return EXIT_FAILURE;
    }

    // PRODUCER (MAIN THREAD)
    rc = pthread_mutex_lock(&mutex);

    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_lock: %s\n",
                strerror(rc));
        return EXIT_FAILURE;
    }

    job = 42;
    ready = 1;

    // Notify consumer
    rc = pthread_cond_signal(&job_available);

    if (rc != 0) {
        fprintf(stderr, "pthread_cond_signal: %s\n",
                strerror(rc));
        (void)pthread_mutex_unlock(&mutex);
        return EXIT_FAILURE;
    }

    rc = pthread_mutex_unlock(&mutex);

    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_unlock: %s\n",
                strerror(rc));
        return EXIT_FAILURE;
    }

    // Wait for consumer
    rc = pthread_join(thread, NULL);

    if (rc != 0) {
        fprintf(stderr, "pthread_join: %s\n",
                strerror(rc));
        return EXIT_FAILURE;
    }

    if (consumer_error != 0) {
        fprintf(stderr, "consumer: %s\n",
                strerror(consumer_error));
        return EXIT_FAILURE;
    }

    puts("Main: consumer finished");

    return EXIT_SUCCESS;
}
