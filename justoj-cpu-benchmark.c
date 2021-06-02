//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2021/1/12.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <ismdeep-c-utils/time.h>

#define MAX_N 500000000ULL

int main() {

    uint64_t total_time = 0ULL;

    for (size_t test_id = 0; test_id < 10; ++test_id) {
        uint64_t start_time_ms = time_ms();

        uint64_t sum = 0;
        for (uint64_t i = 0; i < MAX_N; ++i) {
            sum += i * i;
        }

        uint64_t end_time_ms = time_ms();
        total_time += end_time_ms - start_time_ms;
        SLEEP_S(1);
    }

    uint64_t avg_time = total_time / 10;


    printf("%.3lf\n", 1000.00 / (double) avg_time);

    return EXIT_SUCCESS;
}