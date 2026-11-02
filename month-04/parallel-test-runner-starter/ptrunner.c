#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <spawn.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

#define QUEUE_CAPACITY 8
#define PARALLEL_WORKERS 2

typedef struct {
    char *commands[QUEUE_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
    bool closed;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} TestQueue;

typedef struct {
    TestQueue *queue;
    atomic_uint *consumed;
    size_t worker_id;
} WorkerContext;

typedef struct {
    TestQueue *queue;
    atomic_uint *passed;
    atomic_uint *failed;
    size_t worker_id;
} TestRunContext;

static bool queue_init(TestQueue *q)
{
    memset(q, 0, sizeof(*q));

    int rc = pthread_mutex_init(&q->mutex, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(rc));
        return false;
    }

    rc = pthread_cond_init(&q->not_empty, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_init(not_empty): %s\n", strerror(rc));
        (void)pthread_mutex_destroy(&q->mutex);
        return false;
    }

    rc = pthread_cond_init(&q->not_full, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_init(not_full): %s\n", strerror(rc));
        (void)pthread_cond_destroy(&q->not_empty);
        (void)pthread_mutex_destroy(&q->mutex);
        return false;
    }

    return true;
}

static bool queue_destroy(TestQueue *q)
{
    bool ok = true;
    int rc = pthread_cond_destroy(&q->not_full);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_destroy(not_full): %s\n", strerror(rc));
        ok = false;
    }

    rc = pthread_cond_destroy(&q->not_empty);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_destroy(not_empty): %s\n", strerror(rc));
        ok = false;
    }

    rc = pthread_mutex_destroy(&q->mutex);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_destroy: %s\n", strerror(rc));
        ok = false;
    }

    return ok;
}

/* A successful push transfers ownership of command to the queue. */
static bool queue_push(TestQueue *q, char *command)
{
    pthread_mutex_lock(&q->mutex);

    while (q->count == QUEUE_CAPACITY && !q->closed) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    if (q->closed) {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    q->commands[q->tail] = command;
    q->tail = (q->tail + 1) % QUEUE_CAPACITY;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
    return true;
}

/* A successful pop transfers ownership of *command to the consumer. */
static bool queue_pop(TestQueue *q, char **command)
{
    pthread_mutex_lock(&q->mutex);

    while (q->count == 0 && !q->closed) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    if (q->count == 0 && q->closed) {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    *command = q->commands[q->head];
    q->commands[q->head] = NULL;
    q->head = (q->head + 1) % QUEUE_CAPACITY;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    return true;
}

static void queue_close(TestQueue *q)
{
    pthread_mutex_lock(&q->mutex);
    q->closed = true;
    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
}

static void *queue_worker(void *arg)
{
    WorkerContext *ctx = arg;
    char *command = NULL;

    while (queue_pop(ctx->queue, &command)) {
        printf("Worker %zu: %s\n", ctx->worker_id, command);
        free(command);
        atomic_fetch_add(ctx->consumed, 1u);
    }

    return NULL;
}

static int queue_selftest(void)
{
    enum { WORKER_COUNT = 2, JOB_COUNT = 12, COMMAND_SIZE = 32 };

    TestQueue queue;
    if (!queue_init(&queue)) {
        return EXIT_FAILURE;
    }

    atomic_uint consumed;
    atomic_init(&consumed, 0u);

    WorkerContext contexts[WORKER_COUNT];
    pthread_t workers[WORKER_COUNT];
    size_t started = 0;
    size_t enqueued = 0;
    bool success = true;
    bool joined_all = true;

    for (size_t i = 0; i < WORKER_COUNT; i++) {
        contexts[i] = (WorkerContext) {
            .queue = &queue,
            .consumed = &consumed,
            .worker_id = i
        };

        int rc = pthread_create(&workers[i], NULL, queue_worker, &contexts[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(rc));
            success = false;
            break;
        }
        started++;
    }

    if (success) {
        for (size_t i = 0; i < JOB_COUNT; i++) {
            char *command = malloc(COMMAND_SIZE);
            if (command == NULL) {
                perror("malloc");
                success = false;
                break;
            }

            int n = snprintf(command, COMMAND_SIZE, "command %zu", i);
            if (n < 0 || n >= COMMAND_SIZE) {
                fprintf(stderr, "Command formatting failed\n");
                free(command);
                success = false;
                break;
            }

            if (!queue_push(&queue, command)) {
                fprintf(stderr, "queue_push failed\n");
                free(command);
                success = false;
                break;
            }
            enqueued++;
        }
    }

    queue_close(&queue);

    for (size_t i = 0; i < started; i++) {
        int rc = pthread_join(workers[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(rc));
            success = false;
            joined_all = false;
        }
    }

    unsigned total_consumed = atomic_load(&consumed);
    printf("\nEnqueued: %zu / %d\n", enqueued, JOB_COUNT);
    printf("Consumed: %u / %d\n", total_consumed, JOB_COUNT);

    if (enqueued != JOB_COUNT || total_consumed != JOB_COUNT) {
        success = false;
    }

    /* Destroy synchronization objects only after all threads are joined. */
    if (joined_all && !queue_destroy(&queue)) {
        success = false;
    }

    if (success) {
        puts("QUEUE SELFTEST PASSED");
        return EXIT_SUCCESS;
    }

    fputs("QUEUE SELFTEST FAILED\n", stderr);
    return EXIT_FAILURE;
}

static int run_test(const char *command)
{
    pid_t pid;
    char *const args[] = { "sh", "-c", (char *)command, NULL };

    int rc = posix_spawn(&pid, "/bin/sh", NULL, NULL, args, environ);
    if (rc != 0) {
        fprintf(stderr, "posix_spawn: %s\n", strerror(rc));
        return 125;
    }

    int status;
    for (;;) {
        pid_t result = waitpid(pid, &status, 0);
        if (result == pid) {
            break;
        }
        if (result == -1 && errno == EINTR) {
            continue;
        }
        perror("waitpid");
        return 125;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return 125;
}

static void *test_worker(void *arg)
{
    TestRunContext *ctx = arg;
    char *command = NULL;

    while (queue_pop(ctx->queue, &command)) {
        int result = run_test(command);

        if (result == 0) {
            atomic_fetch_add(ctx->passed, 1u);
            printf("[Worker %zu] PASS: %s\n", ctx->worker_id, command);
        } else {
            atomic_fetch_add(ctx->failed, 1u);
            printf("[Worker %zu] FAIL (%d): %s\n",
                   ctx->worker_id, result, command);
        }

        free(command);
        command = NULL;
    }
    return NULL;
}

static int run_parallel_manifest(const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    TestQueue queue;
    if (!queue_init(&queue)) {
        (void)fclose(file);
        return EXIT_FAILURE;
    }

    atomic_uint passed;
    atomic_uint failed;
    atomic_init(&passed, 0u);
    atomic_init(&failed, 0u);

    TestRunContext contexts[PARALLEL_WORKERS];
    pthread_t workers[PARALLEL_WORKERS];
    size_t started = 0;
    size_t enqueued = 0;
    bool success = true;
    bool joined_all = true;
    char *line = NULL;
    size_t capacity = 0;

    for (size_t i = 0; i < PARALLEL_WORKERS; i++) {
        contexts[i] = (TestRunContext) {
            .queue = &queue,
            .passed = &passed,
            .failed = &failed,
            .worker_id = i
        };

        int rc = pthread_create(&workers[i], NULL, test_worker, &contexts[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(rc));
            success = false;
            break;
        }
        started++;
    }

    if (success) {
        while (getline(&line, &capacity, file) != -1) {
            line[strcspn(line, "\r\n")] = '\0';
            if (line[0] == '\0' || line[0] == '#') {
                continue;
            }

            char *command = strdup(line);
            if (command == NULL) {
                perror("strdup");
                success = false;
                break;
            }

            if (!queue_push(&queue, command)) {
                fprintf(stderr, "queue_push failed\n");
                free(command);
                success = false;
                break;
            }
            enqueued++;
        }

        if (ferror(file)) {
            fprintf(stderr, "Error reading manifest: %s\n", path);
            success = false;
        }
    }

    free(line);
    if (fclose(file) != 0) {
        perror("fclose");
        success = false;
    }

    queue_close(&queue);
    for (size_t i = 0; i < started; i++) {
        int rc = pthread_join(workers[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(rc));
            success = false;
            joined_all = false;
        }
    }

    unsigned total_passed = atomic_load(&passed);
    unsigned total_failed = atomic_load(&failed);
    printf("\nSummary: %u passed, %u failed, %u total\n",
           total_passed, total_failed, total_passed + total_failed);

    if ((size_t)total_passed + (size_t)total_failed != enqueued) {
        fprintf(stderr, "Job accounting mismatch: submitted=%zu, completed=%u\n",
                enqueued, total_passed + total_failed);
        success = false;
    }

    if (joined_all && !queue_destroy(&queue)) {
        success = false;
    }

    if (success && total_failed == 0) {
        puts("All tests passed!");
        return EXIT_SUCCESS;
    }

    puts("Some tests failed or the runner encountered an error.");
    return EXIT_FAILURE;
}

static int run_sequential_manifest(const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    unsigned passed = 0;
    unsigned failed = 0;
    char *line = NULL;
    size_t capacity = 0;

    while (getline(&line, &capacity, file) != -1) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        int result = run_test(line);
        if (result == 0) {
            passed++;
            printf("Test passed: %s\n", line);
        } else {
            failed++;
            fprintf(stderr, "Test failed (%d): %s\n", result, line);
        }
    }

    bool read_error = (ferror(file) != 0);
    free(line);
    if (fclose(file) != 0) {
        perror("fclose");
        return EXIT_FAILURE;
    }
    if (read_error) {
        fprintf(stderr, "Error reading manifest: %s\n", path);
        return EXIT_FAILURE;
    }

    printf("\nSummary: %u passed, %u failed, %u total\n",
           passed, failed, passed + failed);
    if (failed == 0) {
        puts("All tests passed!");
        return EXIT_SUCCESS;
    }
    puts("Some tests failed.");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--queue-selftest") == 0) {
        return queue_selftest();
    }

    if (argc == 3 && strcmp(argv[1], "--parallel") == 0) {
        return run_parallel_manifest(argv[2]);
    }

    if (argc == 2 && argv[1][0] != '-') {
        return run_sequential_manifest(argv[1]);
    }

    fprintf(stderr,
            "Usage: %s <manifest.txt>\n"
            "       %s --parallel <manifest.txt>\n"
            "       %s --queue-selftest\n",
            argv[0], argv[0], argv[0]);
    return EXIT_FAILURE;
}
