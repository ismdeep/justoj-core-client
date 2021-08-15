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


struct Env {
    char *PYTHON2; // PYTHON2
    char *PYTHON3; // PYTHON3
    char *GUILE; // GUILE
    char *SBCL; // SBCL
    char *JAVA; // JAVA
    char *JAVAC; // JAVAC
    char *NODE; // NODE
    char *GO; // GO
    char *PHP; // PHP
    char *PERL; // PERL
    char *LUA; // LUA
    char *LUAC; // LUAC
    char *FPC; // FPC
    char *RUBY; // RUBY
    char *MCS; // MCS
    char *MONO; // MONO
};

struct Env *env_create() {
    struct Env *env = (struct Env *) malloc(sizeof(struct Env) * 1);
    env->PYTHON2 = create_str(1024);
    env->PYTHON3 = create_str(1024);
    env->GUILE = create_str(1024);
    env->SBCL = create_str(1024);
    env->JAVA = create_str(1024);
    env->JAVAC = create_str(1024);
    env->NODE = create_str(1024);
    env->GO = create_str(1024);
    env->PHP = create_str(1024);
    env->PERL = create_str(1024);
    env->LUA = create_str(1024);
    env->LUAC = create_str(1024);
    env->FPC = create_str(1024);
    env->RUBY = create_str(1024);
    env->MCS = create_str(1024);
    env->MONO = create_str(1024);

    return env;
}

struct SolutionInfo {
    int lang_id; // LANGUAGE
    bool is_special_judge; // SPECIAL_JUDGE
    double cpu_compensation; // CPU_COMPENSATION
    int time_lmt; // TIME_LIMIT
    int mem_lmt; // MEMORY_LIMIT


    int result;
    int result_time;
    int result_memory;

    unsigned int call_id;
    unsigned int *call_counter;
};

struct SolutionInfo *solution_info_create() {
    struct SolutionInfo *solution_info = (struct SolutionInfo *) malloc(sizeof(struct SolutionInfo) * 1);
    solution_info->call_counter = (unsigned int *) malloc(sizeof(unsigned int) * CALL_ARRAY_SIZE);

    return solution_info;
}

#endif //JUSTOJ_CORE_SOLUTION_INFO_H
