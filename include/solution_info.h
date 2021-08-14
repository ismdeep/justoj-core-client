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
    int lang_id; // LANGUAGE
    bool is_special_judge; // SPECIAL_JUDGE
    double cpu_compensation; // CPU_COMPENSATION
    int time_lmt; // TIME_LIMIT
    int mem_lmt; // MEMORY_LIMIT
    char *python2_path; // PYTHON2
    char *python3_path; // PYTHON3
    char *guile_path; // GUILE
    char *sbcl_path; // SBCL

    int result;
    int result_time;
    int result_memory;

    unsigned int call_id;
    unsigned int *call_counter;
};

struct SolutionInfo *solution_info_create() {
    struct SolutionInfo *solution_info = (struct SolutionInfo *) malloc(sizeof(struct SolutionInfo) * 1);
    solution_info->python2_path = create_str(1024);
    solution_info->python3_path = create_str(1024);
    solution_info->guile_path = create_str(1024);
    solution_info->sbcl_path = create_str(1024);

    solution_info->call_counter = (unsigned int *) malloc(sizeof(unsigned int) * CALL_ARRAY_SIZE);


    return solution_info;
}

#endif //JUSTOJ_CORE_SOLUTION_INFO_H
