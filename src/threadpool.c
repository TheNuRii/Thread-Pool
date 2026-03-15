#include <stdlib.h>
#include <stdio.h>
#include "../inc/threadpool.h"
#include <unistd.h>

void* thread_function(void* threadpool) {
    threadpool_t* pool = (threadpool_t*)threadpool;
    
    while(1) {
        // Lock the thread pool mutex
        pthread_mutex_lock(&pool->lock);

        while (pool->queued == 0 && !pool->stop) {
            pthread_cond_wait(&pool->notify, &(pool->lock));
        }

        if (pool->stop) {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }

        // Get the next task from the queue
        task_t task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % QUEUE_SIZE;
        pool->queued--;

        (*task.fn)(task.arg); // Execute the task
        pthread_mutex_unlock(&pool->lock);
    }

    return NULL;
}

void threadpool_init(threadpool_t* pool) {
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->notify, NULL);
    pool->queued = 0;
    pool->queue_front = 0;
    pool->queue_back = 0;
    pool->stop = 0;

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&pool->threads[i], NULL, (void*)thread_function, pool);
    }
}
void threadpool_destroy(threadpool_t* pool) {
    pthread_mutex_lock(&pool->lock);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->notify);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);
}

void threadpool_add(threadpool_t* pool, void (*function)(void*), void* arg) {
    pthread_mutex_lock(&pool->lock);

    if (pool->queued < QUEUE_SIZE) {
        pool->task_queue[pool->queue_back].fn = function;
        pool->task_queue[pool->queue_back].arg = arg;
        pool->queue_back = (pool->queue_back + 1) % QUEUE_SIZE;
        pool->queued++;
        pthread_cond_signal(&pool->notify);
    } else {
        fprintf(stderr, "Task queue is full\n");
    }

    pthread_mutex_unlock(&(pool->lock));
}

void example_task(void* arg) {
    int* num = (int*)arg;
    printf("Task executed with argument: %d\n", *num);
    sleep(1); // Simulate work
    free(num);
}