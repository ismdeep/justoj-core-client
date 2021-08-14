//
// Created by L. Jiang on 2020/12/25.
//

#ifndef JUSTOJ_CORE_WATCH_SOLUTIONS_H
#define JUSTOJ_CORE_WATCH_SOLUTIONS_H

#include <sys/user.h>
#include <sys/ptrace.h>
#include <solution_info.h>

int get_proc_status(int pid, const char *mark) {
    char fn[BUFFER_SIZE], buf[BUFFER_SIZE];
    int ret = 0;
    sprintf(fn, "/proc/%d/status", pid);
    FILE *fp = fopen(fn, "re");
    int m = strlen(mark);
    if (!fp) {
        return 0;
    }
    while (fgets(buf, BUFFER_SIZE - 1, fp)) {
        buf[strlen(buf) - 1] = 0;
        if (strncmp(buf, mark, m) == 0) {
            sscanf(buf + m + 1, "%d", &ret);
        }
    }
    fclose(fp);
    return ret;
}

void print_runtime_error(char *err) {
    log_info("Runtime Error: %s", err);
}


int get_page_fault_mem(struct rusage *ruse, const pid_t pid) {
    /* java use pagefault */
    int m_vmpeak, m_vmdata, m_minflt;
    m_minflt = (int) ruse->ru_minflt * getpagesize();
    m_vmpeak = get_proc_status(pid, "VmPeak:");
    m_vmdata = get_proc_status(pid, "VmData:");
    printf("VmPeak:%d KB VmData:%d KB minflt:%d KB\n", m_vmpeak, m_vmdata, m_minflt >> 10);
    return m_minflt;
}

void watch_solution(struct SolutionInfo *solution_info, pid_t pid) {
    char record_call = 0;
    /* parent */

    int status, sig, exitcode;
    struct user_regs_struct reg;
    struct rusage ruse;
    int first = true;
    while (1) {
        /* check the usage */
        wait4(pid, &status, __WALL, &ruse);
        if (first) { //
            ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT);
        }

        // jvm gc ask VM before need,so used kernel page fault times and page size
        if (solution_info->lang_id == 3
            || solution_info->lang_id == 7
            || solution_info->lang_id == 9
            || solution_info->lang_id == 13
            || solution_info->lang_id == 14
            || solution_info->lang_id == 16
            || solution_info->lang_id == 17) {
            solution_info->result_memory = get_page_fault_mem(&ruse, pid);
        } else {        //other use VmPeak
            solution_info->result_memory = get_proc_status(pid, "VmPeak:") << 10;
        }

        if (solution_info->result_memory > solution_info->mem_lmt * STD_MB) {
            if (solution_info->result == OJ_AC)
                solution_info->result = OJ_ML;
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            break;
        }
        /* sig = status >> 8; status >> 8 EXITCODE */
        if (WIFEXITED(status))
            break;
        if ((solution_info->lang_id < 4 || solution_info->lang_id == 9) && get_file_size("error.out")) {
            solution_info->result = OJ_RE;
            /* addreinfo(solution_id); */
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            break;
        }

        exitcode = WEXITSTATUS(status);
/*exitcode == 5 waiting for next CPU allocation          * ruby using system to run,exit 17 ok
 *  Runtime Error:Unknown signal xxx need be added here
 */
        if ((solution_info->lang_id >= 3 && exitcode == 17) || exitcode == 0x05 || exitcode == 0 || exitcode == 133) {
            //go on and on
            ;
        } else {
            //psignal(exitcode, NULL);
            if (solution_info->result == OJ_AC) {
                switch (exitcode) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                        solution_info->result = OJ_TL;
                        solution_info->result_time = solution_info->time_lmt * 1000;
                        break;
                    case SIGKILL:
                    case SIGXCPU:
                        solution_info->result = OJ_TL;
                        solution_info->result_time = solution_info->time_lmt * 1000;
                        break;
                    case SIGXFSZ:
                        solution_info->result = OJ_OL;
                        break;
                    default:
                        solution_info->result = OJ_RE;
                }

                print_runtime_error(strsignal(exitcode));
            }
            ptrace(PTRACE_KILL, pid, NULL, NULL);

            break;
        }
        if (WIFSIGNALED(status)) {
/*  WIFSIGNALED: if the process is terminated by signal
 *
 *  psignal(int sig, char *s)，like perror(char *s)，print out s, with error msg from system of sig
 * sig = 5 means Trace/breakpoint trap
 * sig = 11 means Segmentation fault
 * sig = 25 means File size limit exceeded
 */
            sig = WTERMSIG(status);

            if (solution_info->result == OJ_AC) {
                switch (sig) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                        solution_info->result = OJ_TL;
                        break;
                    case SIGKILL:
                    case SIGXCPU:
                        solution_info->result = OJ_TL;
                        break;
                    case SIGXFSZ:
                        solution_info->result = OJ_OL;
                        break;
                    default:
                        solution_info->result = OJ_RE;
                }

                print_runtime_error(strsignal(sig));

            }
            break;
        }
/*     comment from http://www.felix021.com/blog/read.php?1662

WIFSTOPPED: return true if the process is paused or stopped while ptrace is watching on it
WSTOPSIG: get the signal if it was stopped by signal
 */

// check the system calls
        ptrace(PTRACE_GETREGS, pid, NULL, &reg);
        solution_info->call_id = (unsigned int) reg.REG_SYSCALL % CALL_ARRAY_SIZE;
        if (solution_info->call_counter[solution_info->call_id]) {
//call_counter[reg.REG_SYSCALL]--;
        } else if (record_call) {
            solution_info->call_counter[solution_info->call_id] = 1;
        } else { //do not limit JVM syscall for using different JVM
            solution_info->result = OJ_RE;
            char error[BUFFER_SIZE];
            sprintf(error,
                    "[ERROR] A Not allowed system call [CALL_ID: %u]\n"
                    " TO FIX THIS , ask admin to add the CALL_ID into corresponding LANG_XXV[] located at okcalls32/64.h ,\n"
                    "and recompile justoj_core_client. \n"
                    "if you are admin and you don't know what to do ,\n"
                    "chinese explaination can be found on https://zhuanlan.zhihu.com/p/24498599\n",
                    solution_info->call_id);

            // printf("%s", error);
            print_runtime_error(error);
            ptrace(PTRACE_KILL, pid, NULL, NULL);
        }


        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        first = false;
    }

    int tmp_used_time = 0;

    tmp_used_time += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000) * solution_info->cpu_compensation;
    tmp_used_time += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000) * solution_info->cpu_compensation;

    solution_info->result_time = tmp_used_time;
}

#endif //JUSTOJ_CORE_WATCH_SOLUTIONS_H
