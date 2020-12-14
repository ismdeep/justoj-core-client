//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2020/12/13.
//

#ifndef JUSTOJ_CORE_SOLUTION_QUEUE_H
#define JUSTOJ_CORE_SOLUTION_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct SolutionQueue {
    int                max_size;
    int                size;
    int                *solution_ids;
    int                push_cursor;
    int                pop_cursor;
    pthread_mutex_t    *mutex;
};

struct SolutionQueue *solution_queue_create(int max_size) {
    struct SolutionQueue *queue = (struct SolutionQueue *) malloc(sizeof(struct SolutionQueue) * 1);
    queue->max_size = max_size;
    queue->size = 0;
    queue->solution_ids = (int *) malloc(sizeof(int) * max_size);
    for (int i = 0; i < max_size; i++) {
        queue->solution_ids[i] = 0;
    }
    queue->push_cursor = 0;
    queue->pop_cursor  = 0;
    queue->mutex = NULL;
    return queue;
}

void solution_queue_destroy(struct SolutionQueue *queue) {
    free(queue->solution_ids);
    free(queue);
}

bool solution_queue_push(struct SolutionQueue *queue, int solution_id) {
    size_t index;
    bool flag = false;

    /* mutex lock */
    if (queue->mutex != NULL) pthread_mutex_lock(queue->mutex);

    for (int i = 0; i < queue->max_size; ++i) {
        index = (queue->push_cursor + i) % queue->max_size;
        if (queue->solution_ids[index] == 0) {
            queue->solution_ids[index] = solution_id;
            ++queue->size;
            queue->push_cursor = (index + 1) % queue->max_size;
            flag = true;
            break;
        }
    }

    /* mutex unlock */
    if (queue->mutex != NULL) pthread_mutex_unlock(queue->mutex);

    return flag;
}

int solution_queue_pop(struct SolutionQueue *queue) {
    int solution_id = 0;
    int index;

    /* mutex lock */
    if (queue->mutex != NULL) pthread_mutex_lock(queue->mutex);

    for (int i = 0; i < queue->max_size; ++i) {
        index = (queue->pop_cursor + i) % queue->max_size;
        if (queue->solution_ids[index] != 0) {
            solution_id = queue->solution_ids[index];
            queue->solution_ids[index] = 0;
            queue->pop_cursor = (index + 1) % queue->max_size;
            --queue->size;
            break;
        }
    }

    /* mutex unlock */
    if (queue->mutex != NULL) pthread_mutex_unlock(queue->mutex);

    return solution_id;
}

#endif //JUSTOJ_CORE_SOLUTION_QUEUE_H
