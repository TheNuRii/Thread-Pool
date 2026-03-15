#include "../inc/threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    threadpool_t pool;
    threadpool_init(&pool);

    for (int i = 0; i < 100; i++) {
        int* task_num = malloc(sizeof(int));
        *task_num = i;  
        threadpool_add(&pool, example_task, task_num);
    }

    threadpool_destroy(&pool);
    return 0;
}