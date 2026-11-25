
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Information given to a worker
struct worker_args {
    int id;
};

// Function executed by the worker
static void *worker(void *arg)
{
    const struct worker_args *data = arg;
printf("worker %d: process PID=%ld\n",
           data->id, (long)getpid());
    // TODO 1:
    // Print the worker ID and process PID.

    return NULL;
}
int main(void)
{
    pthread_t threads[2];
    struct worker_args args[2];

    int rc;
    int created = 0;

    // TODO 1:
for(int i=0; i<2; i++){
args[i].id=i;}
    // Use a for loop to initialize:
    // args[0].id = 0
    // args[1].id = 1
for(int i=0; i < 2; i++){
 rc = pthread_create(&threads[i], NULL, worker, &args[i]);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(rc));
break;    }
created++;
    puts("main: worker created");}
for(int i=0; i < created; i++){
rc = pthread_join(threads[i], NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(rc));
        return EXIT_FAILURE;   }

    puts("main: worker joined");}
puts("Main: worker finished successfully");

    return EXIT_SUCCESS;
}

    // TODO 2:
    // Use a for loop to create TWO threads.
    // Use pthread_create().
    // If creation fails, print the error
    // and break out of the loop.
    // Increase 'created' after each success.

    // TODO 3:
    // Use another for loop to join ONLY
    // the successfully created threads.
    // Check each pthread_join() result.

    // TODO 4:
    // Return EXIT_FAILURE if either
    // thread creation or joining failed.
    // Otherwise print a completion message.

  
