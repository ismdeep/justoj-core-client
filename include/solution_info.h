//
// Created by L. Jiang on 2020/12/25.
//

#ifndef JUSTOJ_CORE_SOLUTION_INFO_H
#define JUSTOJ_CORE_SOLUTION_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <str_utils.h>

struct SolutionInfo {
    int solution_id;
    int problem_id;
    char *user_id;

    int lang_id;
    char *src_path;

    int time_lmt;
    int mem_lmt;

    bool is_special_judge;
    int result;
    int result_time;
    int result_memory;

    unsigned int call_id;
    unsigned int *call_counter;
};

struct SolutionInfo *solution_info_create() {
    struct SolutionInfo *solution_info = (struct SolutionInfo *) malloc(sizeof(struct SolutionInfo) * 1);
    solution_info->user_id = create_str(1024);
    solution_info->src_path = create_str(1024);

    solution_info->call_counter = (unsigned  int *) malloc(sizeof(unsigned  int) * CALL_ARRAY_SIZE);

    return solution_info;
}

#endif //JUSTOJ_CORE_SOLUTION_INFO_H
