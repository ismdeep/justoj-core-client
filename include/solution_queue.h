//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2020/12/13.
//

#ifndef JUSTOJ_CORE_SOLUTION_QUEUE_H
#define JUSTOJ_CORE_SOLUTION_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct SolutionQueue {
    size_t max_size;
    size_t size;
    size_t *solution_ids;
    size_t push_cursor;
    size_t pop_cursor;
};

struct SolutionQueue *solution_queue_create(size_t max_size) {
    struct SolutionQueue *queue = (struct SolutionQueue *) malloc(sizeof(struct SolutionQueue) * 1);
    queue->max_size = max_size;
    queue->size = 0;
    queue->solution_ids = (size_t *) malloc(sizeof(size_t) * max_size);
    queue->push_cursor = 0;
    queue->pop_cursor  = 0;
    return queue;
}

void solution_queue_destroy(struct SolutionQueue *queue) {
    free(queue->solution_ids);
    free(queue);
}

void solution_queue_push(struct SolutionQueue *queue, size_t solution_id) {
    size_t index;
    for (size_t i = 0; i < queue->max_size; ++i) {
        index = (queue->push_cursor + i) % queue->max_size;
        if (queue->solution_ids[index] == 0) {
            queue->solution_ids[index] = solution_id;
            ++queue->size;
            queue->push_cursor = (index + 1) % queue->max_size;
            return;
        }
    }
}

size_t solution_queue_pop(struct SolutionQueue *queue) {
    size_t solution_id = 0;
    size_t index;
    for (size_t i = 0; i < queue->max_size; ++i) {
        index = (queue->pop_cursor + i) % queue->max_size;
        if (queue->solution_ids[index] != 0) {
            solution_id = queue->solution_ids[index];
            queue->solution_ids[index] = 0;
            queue->pop_cursor = (index + 1) % queue->max_size;
            --queue->size;
            break;
        }
    }
    return solution_id;
}


#endif //JUSTOJ_CORE_SOLUTION_QUEUE_H
