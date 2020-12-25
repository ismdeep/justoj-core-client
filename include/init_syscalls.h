//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2020/12/25.
//

#ifndef JUSTOJ_CORE_INIT_SYSCALLS_H
#define JUSTOJ_CORE_INIT_SYSCALLS_H

#include "okcalls.h"

void init_syscall_limits(struct SolutionInfo *solution_info) {
    int i;
    memset(solution_info->call_counter, 0, sizeof(unsigned int) * CALL_ARRAY_SIZE);
    switch (solution_info->lang_id) {
        case LANG_C:
        case LANG_CXX:
        case LANG_CLANG:
        case LANG_CLANG_XX:
            for (i = 0; i == 0 || LANG_CV[i]; i++) solution_info->call_counter[LANG_CV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PASCAL:
            for (i = 0; i == 0 || LANG_PV[i]; i++) solution_info->call_counter[LANG_PV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_JAVA:
            for (i = 0; i == 0 || LANG_JV[i]; i++) solution_info->call_counter[LANG_JV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_RUBY:
            for (i = 0; i == 0 || LANG_RV[i]; i++) solution_info->call_counter[LANG_RV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_BASH:
            for (i = 0; i == 0 || LANG_BV[i]; i++) solution_info->call_counter[LANG_BV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PYTHON2:
            for (i = 0; i == 0 || LANG_YV[i]; i++) solution_info->call_counter[LANG_YV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PHP:
            for (i = 0; i == 0 || LANG_PHV[i]; i++) solution_info->call_counter[LANG_PHV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PERL:
            for (i = 0; i == 0 || LANG_PLV[i]; i++) solution_info->call_counter[LANG_PLV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_C_SHARP:
            for (i = 0; i == 0 || LANG_CSV[i]; i++) solution_info->call_counter[LANG_CSV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_OBJC:
            for (i = 0; i == 0 || LANG_OV[i]; i++) solution_info->call_counter[LANG_OV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_FREE_BASIC:
            for (i = 0; i == 0 || LANG_BASICV[i]; i++) solution_info->call_counter[LANG_BASICV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_SCHEME:
            for (i = 0; i == 0 || LANG_SV[i]; i++) solution_info->call_counter[LANG_SV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_SBCL:
            for (i = 0; i == 0 || LANG_SBCLV[i]; i++) solution_info->call_counter[LANG_SBCLV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_LUA:
            for (i = 0; i == 0 || LANG_LUAV[i]; i++) solution_info->call_counter[LANG_LUAV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_JAVASCRIPT:
            for (i = 0; i == 0 || LANG_JSV[i]; i++) solution_info->call_counter[LANG_JSV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_GO:
            for (i = 0; i == 0 || LANG_GOV[i]; i++) solution_info->call_counter[LANG_GOV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PYTHON3:
            for (i = 0; i == 0 || LANG_YV[i]; i++) solution_info->call_counter[LANG_YV[i]] = HOJ_MAX_LIMIT;
            break;
        default:
            printf("ERROR: Unsupport Language.\n");
    }
}

#endif //JUSTOJ_CORE_INIT_SYSCALLS_H
