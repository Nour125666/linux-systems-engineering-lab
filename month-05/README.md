#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

#define CAPACITY 2

typedef struct {
    int jobs[CAPACITY];
    size_t count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
} Queue;

static void queue_push(Queue *queue, int job)
{
    pthread_mutex_lock(&queue->mutex);
    while (queue->count == CAPACITY) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    queue->jobs[queue->count++] = job;
    pthread_mutex_unlock(&queue->mutex);
}

int main(void)
{
    Queue queue = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .not_full = PTHREAD_COND_INITIALIZER
    };

    queue_push(&queue, 1);
    queue_push(&queue, 2);
    puts("Trying to enqueue third job...");
    fflush(stdout);
    queue_push(&queue, 3);
    puts("All jobs enqueued");

    pthread_cond_destroy(&queue.not_full);
    pthread_mutex_destroy(&queue.mutex);
    return 0;
}
