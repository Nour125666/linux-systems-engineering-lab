
struct worker_status {
    int error;
};

static void *worker(void *arg)
{
    struct worker_status *status = arg;
    status->error = 0;

    for (int i = 0; i < ITERATIONS; i++) {
        int rc;

        // TODO 1:
        // Lock counter_mutex.

        // TODO 2:
        // Check the lock return value.
        // If it fails, save rc in status->error
        // and return NULL.

        // TODO 3:
        // Increment counter.

        // TODO 4:
        // Unlock counter_mutex.
        // Check the return value and save
        // any error in status->error.
    }

    return NULL;
}
