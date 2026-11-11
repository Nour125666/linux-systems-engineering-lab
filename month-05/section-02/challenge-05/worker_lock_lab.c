
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t gate;

struct worker_config {
    const char *name;
    pthread_mutex_t *first;
    pthread_mutex_t *second;
};

static void check(int rc, const char *action)
{
    if (rc != 0) {
        fprintf(stderr, "%s: %s\n", action, strerror(rc));
        exit(EXIT_FAILURE);
    }
}

static void *run_worker(void *arg)
{
    const struct worker_config *cfg = arg;

    check(pthread_mutex_lock(cfg->first), "first lock");

    fprintf(stderr, "%s: stage 1 complete\n", cfg->name);

    int rc = pthread_barrier_wait(&gate);
    if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
        check(rc, "barrier");

    fprintf(stderr, "%s: beginning stage 2\n", cfg->name);

    check(pthread_mutex_lock(cfg->second), "second lock");

    fprintf(stderr, "%s: request complete\n", cfg->name);

    check(pthread_mutex_unlock(cfg->second), "second unlock");
    check(pthread_mutex_unlock(cfg->first), "first unlock");

    return NULL;
}

int main(void)
{
    pthread_t workers[2];

    struct worker_config cfg[2] = {
        {"collector", &config_mutex, &io_mutex},
        {"publisher", &io_mutex, &config_mutex}
    };

    check(pthread_barrier_init(&gate, NULL, 2), "barrier init");

    fprintf(stderr, "Supervisor PID=%ld\n", (long)getpid());

    for (int i = 0; i < 2; ++i)
        check(pthread_create(&workers[i], NULL,
                             run_worker, &cfg[i]), "pthread_create");

    for (int i = 0; i < 2; ++i)
        check(pthread_join(workers[i], NULL), "pthread_join");

    check(pthread_barrier_destroy(&gate), "barrier destroy");
    check(pthread_mutex_destroy(&config_mutex), "config destroy");
    check(pthread_mutex_destroy(&io_mutex), "io destroy");

    fprintf(stderr, "Supervisor: all work completed\n");

    return EXIT_SUCCESS;
}
