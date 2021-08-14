//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2021/1/12.
//

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <ismdeep-c-utils/time.h>

#define MAX_N 500000000ULL

#define TOTAL_CNT 20
#define REMOVE_SIZE 5


int64_t cmp_func(const void *a, const void *b) {
    return (*(int64_t *) a - *(int64_t *) b);
}

int main() {

    uint64_t total_time = 0ULL;

    uint64_t time_elapse[TOTAL_CNT];

    for (size_t test_id = 0; test_id < TOTAL_CNT; ++test_id) {
        uint64_t start_time_ms = time_ms();

        uint64_t sum = 0;
        for (uint64_t i = 0; i < MAX_N; ++i) {
            sum += i * i;
        }

        uint64_t end_time_ms = time_ms();
        time_elapse[test_id] = end_time_ms - start_time_ms;
        SLEEP_MS(100);
    }

    qsort(time_elapse, TOTAL_CNT, sizeof(uint64_t), (int (*)(const void *, const void *)) cmp_func);

    int size = 0;
    for (size_t test_id = REMOVE_SIZE; test_id < TOTAL_CNT - REMOVE_SIZE; ++test_id) {
        total_time += time_elapse[test_id];
        ++size;
    }


    uint64_t avg_time = total_time / size;


    printf("%.3lf\n", 1000.00 / (double) avg_time);

    return EXIT_SUCCESS;
}