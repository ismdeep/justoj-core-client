//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2019/12/18.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/resource.h>
#include <okcalls.h>

#include <utils.h>
#include <judge_http_api.h>


#include <ismdeep-c-utils/argv.h>
#include <version.h>

#include <defines.h>

#include <log.h>

#include <system_info.h>
#include <solution_info.h>

#include <init_syscalls.h>
#include <compile.h>

#include <watch_solutions.h>

static int java_time_bonus = 5;
static int java_memory_bonus = 512;
static int full_diff = 0;
static int use_max_time = 0;
static int use_ptrace = 1;

struct SystemInfo *system_info;
struct SolutionInfo *solution_info;

/* Read Judge config */
void init_judge_conf() {
    FILE *fp = NULL;
    char config_file_path[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    strcpy(system_info->java_xms, "-Xms32m");
    strcpy(system_info->java_xmx, "-Xmx256m");
    sprintf(config_file_path, "%s/etc/judge.conf", system_info->oj_home);
    fp = fopen(config_file_path, "re");
    if (fp != NULL) {
        while (fgets(buf, BUFFER_SIZE - 1, fp)) {
            read_int(buf, "OJ_JAVA_TIME_BONUS", &java_time_bonus);
            read_int(buf, "OJ_JAVA_MEMORY_BONUS", &java_memory_bonus);
            read_buf(buf, "OJ_JAVA_XMS", system_info->java_xms);
            read_buf(buf, "OJ_JAVA_XMX", system_info->java_xmx);
            read_buf(buf, "OJ_HTTP_BASE_URL", system_info->http_base_url);
            read_buf(buf, "OJ_SECURE_CODE", system_info->secure_code);
            read_buf(buf, "OJ_CLIENT_NAME", system_info->client_name);
            read_int(buf, "OJ_FULL_DIFF", &full_diff);
            read_int(buf, "OJ_USE_MAX_TIME", &use_max_time);
            read_int(buf, "OJ_USE_PTRACE", &use_ptrace);
            read_double(buf, "OJ_CPU_COMPENSATION", &system_info->cpu_compensation);
            read_buf(buf, "PYTHON2", system_info->python2_path);
            read_buf(buf, "PYTHON3", system_info->python3_path);
            read_buf(buf, "GUILE", system_info->guile_path);
            read_buf(buf, "SCHEME", system_info->scheme_path);
            read_buf(buf, "SBCL", system_info->sbcl_path);
            read_buf(buf, "NODEJS", system_info->nodejs_path);
            read_buf(buf, "DATA", system_info->data_path);
        }
        fclose(fp);
    } else {
        log_info("FILE NOT FOUND. [%s]", config_file_path);
        exit(-1);
    }
}

void push_solution_result() {
    update_solution(system_info, solution_info);
    log_info("==> [%s] %d [LANG:%d] %s",
             system_info->client_name,
             solution_info->solution_id,
             solution_info->lang_id,
             result_text[solution_info->result]);
}


void prepare_files(char *filename, int namelen, char *infile, char *outfile, char *userfile) {
    char fname0[BUFFER_SIZE];
    char fname[BUFFER_SIZE];
    strncpy(fname0, filename, namelen);
    fname0[namelen] = 0;
    escape(fname, fname0);

    execute_cmd("/bin/cp \"%s/%d/%s.in\" %s/run%d/data.in",
                system_info->data_path,
                solution_info->problem_id,
                fname,
                system_info->oj_home,
                solution_info->solution_id);

    execute_cmd("/bin/cp \"%s/%d/*.dic\" %s/ 2> /dev/null",
                system_info->data_path,
                solution_info->problem_id,
                system_info->work_dir);

    sprintf(outfile, "%s/%d/%s.out", system_info->data_path, solution_info->problem_id, fname0);
    sprintf(userfile, "%s/run%d/user.out", system_info->oj_home, solution_info->solution_id);
}

void run_solution(const int *usedtime) {
    nice(19);
    // now the user is "judger"
    chdir(system_info->work_dir);
    // open the files
    freopen("data.in", "r", stdin);
    freopen("user.out", "w", stdout);
    freopen("error.out", "a+", stderr);
    // trace me
    if (use_ptrace)
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    while (setgid(1536) != 0) sleep(1);
    while (setuid(1536) != 0) sleep(1);
    while (setreuid(1536, 1536) != 0) sleep(1);

    struct rlimit LIM; // time limit, file limit& memory limit
    LIM.rlim_cur = (solution_info->time_lmt / system_info->cpu_compensation - *usedtime / 1000) + 1;
    LIM.rlim_max = LIM.rlim_cur;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(0);
    alarm(solution_info->time_lmt * 5 / system_info->cpu_compensation);

    // file limit
    LIM.rlim_max = STD_F_LIM + STD_MB;
    LIM.rlim_cur = STD_F_LIM;
    setrlimit(RLIMIT_FSIZE, &LIM);
    // proc limit
    switch (solution_info->lang_id) {
        case LANG_GO:
        case LANG_C_SHARP:
        case LANG_JAVA:
            LIM.rlim_cur = LIM.rlim_max = 880;
            break;
        case LANG_RUBY:
        case LANG_PYTHON2:
        case LANG_PYTHON3:
        case LANG_JAVASCRIPT:
            LIM.rlim_cur = LIM.rlim_max = 200;
            break;
        case LANG_SCHEME:
        case LANG_SBCL:
            LIM.rlim_cur = LIM.rlim_max = 1;
            break;
        case LANG_BASH:
            LIM.rlim_cur = LIM.rlim_max = 3;
            break;
        default:
            LIM.rlim_cur = LIM.rlim_max = 1;
    }

    setrlimit(RLIMIT_NPROC, &LIM);

    // set the stack
    LIM.rlim_cur = STD_MB << 7;
    LIM.rlim_max = STD_MB << 7;
    setrlimit(RLIMIT_STACK, &LIM);
    // set the memory
    LIM.rlim_cur = STD_MB * solution_info->mem_lmt / 2 * 3;
    LIM.rlim_max = STD_MB * solution_info->mem_lmt * 2;

    if (
            LANG_C == solution_info->lang_id
            || LANG_CXX == solution_info->lang_id
            || LANG_PASCAL == solution_info->lang_id
            || LANG_OBJC == solution_info->lang_id
            || LANG_CLANG == solution_info->lang_id
            || LANG_CLANG_XX == solution_info->lang_id) {
        setrlimit(RLIMIT_AS, &LIM);
    }


    switch (solution_info->lang_id) {
        case LANG_C:
        case LANG_CXX:
        case LANG_PASCAL:
        case LANG_OBJC:
        case LANG_FREE_BASIC:
        case LANG_CLANG:
        case LANG_CLANG_XX:
        case LANG_GO:
            execl("./Main", "./Main", (char *) NULL);
            break;
        case LANG_JAVA:
            sprintf(system_info->java_xms, "-Xmx%dM", solution_info->mem_lmt);
            execl("/usr/bin/java", "/usr/bin/java", system_info->java_xms, system_info->java_xmx,
                  "-Djava.security.manager",
                  "-Djava.security.policy=./java.policy", "Main", (char *) NULL);
            break;
        case LANG_RUBY:
            execl("/ruby", "/ruby", "Main.rb", (char *) NULL);
            break;
        case LANG_BASH:
            execl("/bin/bash", "/bin/bash", "Main.sh", (char *) NULL);
            break;
        case LANG_PYTHON2:
            execl(system_info->python2_path, system_info->python2_path, "Main.py",
                  (char *) NULL);
            break;
        case LANG_PHP:
            execl("/php", "/php", "Main.php", (char *) NULL);
            break;
        case LANG_PERL:
            execl("/usr/bin/perl", "/usr/bin/perl", "Main.pl", (char *) NULL);
            break;
        case LANG_C_SHARP:
            execl("/mono", "/mono", "--debug", "Main.exe", (char *) NULL);
            break;
        case LANG_SCHEME:
            execl(system_info->guile_path, system_info->guile_path, "Main.scm", (char *) NULL);
            break;
        case LANG_SBCL:
            execl(system_info->sbcl_path, system_info->sbcl_path, "--script", "Main.cl", (char *) NULL);
            break;
        case LANG_LUA:
            execl("/lua", "/lua", "Main", (char *) NULL);
            break;
        case LANG_JAVASCRIPT:
            execl("/nodejs", "/nodejs", "Main.js", (char *) NULL);
            break;
        case LANG_PYTHON3:
            execl(system_info->python3_path, system_info->python3_path, "Main.py", (char *) NULL);
            break;
    }
    fflush(stderr);
    exit(0);
}

int fix_python_mis_judge(char *work_dir, int *ACflg, int *topmemory, int mem_lmt) {
    int comp_res;
    comp_res = execute_cmd("/bin/grep 'MemoryError'  %s/error.out", work_dir);

    if (!comp_res) {
        log_info("Python need more Memory!");
        *ACflg = OJ_ML;
        *topmemory = mem_lmt * STD_MB;
    }

    return comp_res;
}

int fix_java_mis_judge(char *work_dir, int *ACflg, int *topmemory, int mem_lmt) {
    int comp_res;

    execute_cmd("chmod 700 %s/error.out", work_dir);
    comp_res = execute_cmd("/bin/grep 'Exception'  %s/error.out", work_dir);
    if (!comp_res) {
        log_info("Exception reported\n");
        *ACflg = OJ_RE;
    }
    execute_cmd("cat %s/error.out", work_dir);

    comp_res = execute_cmd(
            "/bin/grep 'java.lang.OutOfMemoryError'  %s/error.out", work_dir);

    if (!comp_res) {
        log_info("JVM need more Memory!");
        *ACflg = OJ_ML;
        *topmemory = mem_lmt * STD_MB;
    }

    if (!comp_res) {
        log_info("JVM need more Memory or Threads!");
        *ACflg = OJ_ML;
        *topmemory = mem_lmt * STD_MB;
    }
    comp_res = execute_cmd("/bin/grep 'Could not create'  %s/error.out",
                           work_dir);
    if (!comp_res) {
        log_info("jvm need more resource,tweak -Xmx(OJ_JAVA_BONUS) Settings");
        *ACflg = OJ_RE;
        //topmemory=0;
    }
    return comp_res;
}

int special_judge(char *infile, char *outfile, char *userfile) {

    pid_t pid;
    log_info("pid=%d\n", solution_info->problem_id);
    pid = fork();
    int ret = 0;
    if (pid == 0) {

        while (setgid(1536) != 0) sleep(1);
        while (setuid(1536) != 0) sleep(1);
        while (setreuid(1536, 1536) != 0) sleep(1);

        struct rlimit LIM; // time limit, file limit& memory limit

        LIM.rlim_cur = 5;
        LIM.rlim_max = LIM.rlim_cur;
        setrlimit(RLIMIT_CPU, &LIM);
        alarm(0);
        alarm(10);

        // file limit
        LIM.rlim_max = STD_F_LIM + STD_MB;
        LIM.rlim_cur = STD_F_LIM;
        setrlimit(RLIMIT_FSIZE, &LIM);

        ret = execute_cmd("%s/%d/spj '%s' '%s' %s",
                          system_info->data_path,
                          solution_info->problem_id,
                          infile, outfile, userfile);
        if (ret)
            exit(1);
        else
            exit(0);
    } else {
        int status;

        waitpid(pid, &status, 0);
        ret = WEXITSTATUS(status);
    }

    return ret;
}

void judge_solution(char *infile, char *outfile, char *userfile, int num_of_test) {
    if (solution_info->result != OJ_AC) {
        return;
    }

    if (solution_info->result_time > solution_info->time_lmt * 1000 * (use_max_time ? 1 : num_of_test)) {
        solution_info->result = OJ_TL;
        return;
    }
    if (solution_info->result_memory > solution_info->mem_lmt * STD_MB) {
        solution_info->result = OJ_ML;
        return;
    }

    if (solution_info->is_special_judge) {
        solution_info->result = special_judge(infile, outfile, userfile) == 0 ? OJ_AC : OJ_WA;
    } else {
        solution_info->result = compare(outfile, userfile);
    }
}

/**
 * main() function
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {
    if (argv_exist_switch(argc, argv, "--version")) {
        printf("%s version: %s\n", argv[0], get_version());
        return EXIT_SUCCESS;
    }

    /* 0. 判断命令行参数个数是否正确 */
    if (argc < 3) {
        printf("VERSION: %s\n", get_version());
        fprintf(stderr, "Usage: %s ${base_path} ${solution_id}\n", argv[0]);
        exit(1);
    }


    /* 创建系统信息 */
    system_info = system_info_create();
    solution_info = solution_info_create();

    /* 1. 读取命令行参数 */
    strcpy(system_info->oj_home, argv[1]);
    solution_info->solution_id = atoi(argv[2]);

    sprintf(system_info->work_dir, "%s/run%d", system_info->oj_home, solution_info->solution_id);

    char run_work_dir[50];
    sprintf(run_work_dir, "run%d", solution_info->solution_id);

    char log_file_path[1024];
    sprintf(log_file_path, "%s/product.log", system_info->oj_home);
    FILE *log_file = fopen(log_file_path, "ab");
    log_add_fp(log_file, LOG_INFO);
    log_set_quiet(false);

    /* 2. 读取配置文件 */
    init_judge_conf();

    /* 3. 初始化环境 */
    chdir(system_info->oj_home);
    execute_cmd("/bin/mkdir -p %s", run_work_dir);
    execute_cmd("chmod -R 777 run%d", solution_info->solution_id);
    chdir(system_info->work_dir);

    /* 4. 获取Solution信息 */
    get_solution_info(system_info, solution_info);

    /* 5. 获取题目信息 */
    get_problem_info(system_info, solution_info);

    /* 6. 获取Solution源代码 */
    download_source_code(system_info, solution_info);
    execute_cmd("chown judge %s/%s", system_info->work_dir, solution_info->src_path);

    /* 7. 编译代码 */
    if (compile(system_info, solution_info->lang_id) != 0) {
        upload_ce_info(system_info, solution_info);
        solution_info->result = OJ_CE;
        solution_info->result_time = 0;
        solution_info->result_memory = 0;
        push_solution_result();
        /* 清理环境 */
        chdir(system_info->oj_home);
        execute_cmd("rm -rf run%d", solution_info->solution_id);
        exit(0);
    }

    /* 8. 配置时间超限、内存超限 */
    //java is lucky
    if (solution_info->lang_id >= 3 && solution_info->lang_id != 10 && solution_info->lang_id != 13 &&
        solution_info->lang_id != 14) {  // Clang Clang++ not VM or Script
        // the limit for java
        solution_info->time_lmt = solution_info->time_lmt + java_time_bonus;
        solution_info->mem_lmt = solution_info->mem_lmt + java_memory_bonus;
        // copy java.policy
        if (solution_info->lang_id == 3) {
            execute_cmd("/bin/cp %s/etc/java0.policy %s/java.policy 2> /dev/null", system_info->oj_home,
                        system_info->work_dir);
            execute_cmd("chmod 755 %s/java.policy 2> /dev/null", system_info->work_dir);
            execute_cmd("chown judge %s/java.policy 2> /dev/null", system_info->work_dir);
        }
    }
    if (solution_info->time_lmt > 300 || solution_info->time_lmt < 1)
        solution_info->time_lmt = 300;
    if (solution_info->mem_lmt > 1024 || solution_info->mem_lmt < 1)
        solution_info->mem_lmt = 1024;

    /* 9. 设置为正在运行状态 */
    solution_info->result = OJ_RI;
    solution_info->result_time = 0;
    solution_info->result_memory = 0;
    push_solution_result();

    char *data_path = malloc(sizeof(char) * (strlen(system_info->oj_home) + 255));
    char *in_file = malloc(sizeof(char) * (strlen(system_info->oj_home) + 255));
    char *out_file = malloc(sizeof(char) * (strlen(system_info->oj_home) + 255));
    char *user_file = malloc(sizeof(char) * (strlen(system_info->oj_home) + 255));
    sprintf(data_path, "%s/%d", system_info->data_path, solution_info->problem_id);

    // open DIRs
    DIR *dp;
    struct dirent *dirp;

    // using http to get remote test data files
    if (solution_info->problem_id > 0 && (dp = opendir(data_path)) == NULL) {
        log_info("No such dir:%s!", data_path);
        chdir(system_info->oj_home);
        execute_cmd("rm -rf run%d", solution_info->solution_id);
        solution_info->result = OJ_RE;
        solution_info->result_time = 0;
        solution_info->result_memory = 0;
        push_solution_result();
        exit(-1);
    }

    int name_len;
    int user_time = 0;
    int top_memory = 0;
    int num_of_test = 0;


    /* 10. 运行所有测试数据 */
    solution_info->result = OJ_AC;
    int max_case_time = 0;
    while (solution_info->result == OJ_AC && (dirp = readdir(dp)) != NULL) {
        name_len = isInFile(dirp->d_name); // check if the file is *.in or not
        if (name_len == 0)
            continue;
        prepare_files(dirp->d_name, name_len, in_file, out_file, user_file);

        init_syscall_limits(solution_info);
        pid_t pidApp = fork();
        if (pidApp == 0) {
            run_solution(&user_time);
        } else {
            num_of_test++;
            watch_solution(system_info, solution_info, pidApp, in_file, user_file, out_file);
            judge_solution(in_file, out_file, user_file, num_of_test);
            if (use_max_time) {
                max_case_time = user_time > max_case_time ? user_time : max_case_time;
                user_time = 0;
            }
        }
    }

    /* 11. 上传测试结果 */
    if (use_max_time) user_time = max_case_time;
    if (solution_info->result == OJ_TL) user_time = solution_info->time_lmt * 1000;
    solution_info->result_time = user_time;
    solution_info->result_memory = top_memory >> 10;
    push_solution_result();

    /* 清理环境 */
    chdir(system_info->oj_home);
    execute_cmd("rm -rf run%d", solution_info->solution_id);
    free(data_path);
    free(in_file);
    free(out_file);
    free(user_file);
    closedir(dp);

    return 0;
}
