
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 3
#define JOBS 8

// Shared circular queue
static int queue[CAPACITY];
static int head = 0;
static int tail = 0;
static int count = 0;

// Synchronization
static pthread_mutex_t mutex =
    PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t not_empty =
    PTHREAD_COND_INITIALIZER;

static pthread_cond_t not_full =
    PTHREAD_COND_INITIALIZER;

// Simple error handler for this exercise
static void check(int rc, const char *what)
{
    if (rc != 0) {
        fprintf(stderr, "%s: %s\n",
                what, strerror(rc));
        exit(EXIT_FAILURE);
    }
}

// CONSUMER
static void *consumer(void *arg)
{
    (void)arg;

    for (int i = 0; i < JOBS; i++) {

        check(pthread_mutex_lock(&mutex),
              "mutex lock");

        while (count == 0) {
            check(pthread_cond_wait(&not_empty, &mutex),
                  "wait not_empty");
        }

        int job;

        // TODO 1:
        // Read queue[head] into job.
        job = queue[head];
        head = (head + 1) % CAPACITY;
        count--;

        check(pthread_cond_signal(&not_full),
              "signal not_full");

        check(pthread_mutex_unlock(&mutex),
              "mutex unlock");

        printf("Consumer processed job %d\n", job);
    }

    return NULL;
}

int main(void)
{
    pthread_t thread;

    check(pthread_create(&thread, NULL,
                         consumer, NULL),
          "pthread_create");

    // PRODUCER
    for (int job = 1; job <= JOBS; job++) {

        check(pthread_mutex_lock(&mutex),
              "mutex lock");

        while (count == CAPACITY) {
            check(pthread_cond_wait(&not_full, &mutex),
                  "wait not_full");
        }

        queue[tail] = job;
        tail = (tail + 1) % CAPACITY;
        count++;

        printf("Producer queued job %d (count=%d)\n",
               job, count);

        check(pthread_cond_signal(&not_empty),
              "signal not_empty");

        check(pthread_mutex_unlock(&mutex),
              "mutex unlock");
    }

    check(pthread_join(thread, NULL),
          "pthread_join");

    puts("All jobs processed");

    return EXIT_SUCCESS;
}
