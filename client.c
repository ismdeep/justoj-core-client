//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2019/12/18.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <utils.h>
#include <ismdeep-c-utils/argv.h>
#include <version.h>
#include <defines.h>
#include <log.h>
#include <solution_info.h>
#include <init_syscalls.h>
#include <watch_solutions.h>

const char *solution_path;

char java_xms[50] = "-Xms32m";
char java_xmx[50] = "-Xmx256m";
static int java_time_bonus = 5;
static int java_memory_bonus = 512;
char run_dir[1024];
char run_data_dir[1024];
struct SolutionInfo *solution_info;
struct Env *env;

/* Read Judge config */
void init_data() {
    sprintf(run_dir, "%s/run", solution_path);
    sprintf(run_data_dir, "%s/run/data", solution_path);

    solution_info = solution_info_create();
    env = env_create();

    FILE *fp = NULL;
    char config_file_path[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    sprintf(config_file_path, "%s/config", solution_path);
    fp = fopen(config_file_path, "re");
    if (fp != NULL) {
        while (fgets(buf, BUFFER_SIZE - 1, fp)) {
            read_int(buf, "LANGUAGE", &solution_info->lang_id);
            read_double(buf, "CPU_COMPENSATION", &solution_info->cpu_compensation);
            read_int(buf, "TIME_LIMIT", &solution_info->time_lmt);
            read_int(buf, "MEMORY_LIMIT", &solution_info->mem_lmt);
        }
        fclose(fp);
    } else {
        printf("FILE NOT FOUND. [%s]", config_file_path);
        exit(-1);
    }

    FILE *env_fp = fopen("/Env", "re");
    if (env_fp != NULL) {
        while (fgets(buf, BUFFER_SIZE - 1, env_fp)) {
            read_buf(buf, "PYTHON2=", env->PYTHON2);
            read_buf(buf, "PYTHON3=", env->PYTHON3);
            read_buf(buf, "GUILE=", env->GUILE);
            read_buf(buf, "SBCL=", env->SBCL);
            read_buf(buf, "JAVA=", env->JAVA);
            read_buf(buf, "JAVAC=", env->JAVAC);
            read_buf(buf, "NODE=", env->NODE);
            read_buf(buf, "GO=", env->GO);
            read_buf(buf, "PHP=", env->PHP);
            read_buf(buf, "PERL=", env->PERL);
            read_buf(buf, "LUA=", env->LUA);
            read_buf(buf, "LUAC=", env->LUAC);
            read_buf(buf, "FPC=", env->FPC);
            read_buf(buf, "RUBY=", env->RUBY);
            read_buf(buf, "MCS=", env->MCS);
            read_buf(buf, "MONO=", env->MONO);
        }
        fclose(env_fp);
    } else {
        printf("FILE NOT FOUND. [/Env]");
        exit(-1);
    }

    // set time and memory bonus for vm languages
    if (solution_info->lang_id == LANG_JAVA
        || solution_info->lang_id == LANG_RUBY
        || solution_info->lang_id == LANG_BASH
        || solution_info->lang_id == LANG_PYTHON2
        || solution_info->lang_id == LANG_PHP
        || solution_info->lang_id == LANG_PERL
        || solution_info->lang_id == LANG_C_SHARP
        || solution_info->lang_id == LANG_FREE_BASIC
        || solution_info->lang_id == LANG_SCHEME
        || solution_info->lang_id == LANG_LUA
        || solution_info->lang_id == LANG_JAVASCRIPT
        || solution_info->lang_id == LANG_GO
        || solution_info->lang_id == LANG_PYTHON3
        || solution_info->lang_id == LANG_SBCL) {
        solution_info->time_lmt += java_time_bonus;
        solution_info->mem_lmt += java_memory_bonus;
    }

    // copy java.policy
    if (solution_info->lang_id == LANG_JAVA) {
        execute_cmd("chmod 755   %s/run/java.policy 2> /dev/null", solution_path);
        execute_cmd("chown judge %s/run/java.policy 2> /dev/null", solution_path);
    }

    // CPU压缩比
    if (solution_info->cpu_compensation < 0.002) {
        solution_info->cpu_compensation = 1.0;
    }

    sprintf(java_xms, "-Xms%dm", solution_info->mem_lmt);
    sprintf(java_xmx, "-Xmx%dm", solution_info->mem_lmt * 2);

    // 准备数据
    execute_cmd("rm       -rf       %s/run/", solution_path);
    execute_cmd("mkdir    -p        %s/run/", solution_path);
    execute_cmd("mkdir    -p        %s/run/data/", solution_path);
    execute_cmd("cp       -r        %s/data/* %s/run/data/", solution_path, solution_path);
    execute_cmd("chmod    -R 777    %s/run/data", solution_path);
    execute_cmd("cp %s/code %s/run/Main.%s", solution_path, solution_path, lang_ext[solution_info->lang_id]);
}

void run_solution() {
    nice(19);
    // now the user is "judge user"
    chdir(run_dir);
    // open the files
    freopen("data.in", "r", stdin);
    freopen("user.out", "w", stdout);
    freopen("error.out", "a+", stderr);
    while (setgid(1536) != 0) sleep(1);
    while (setuid(1536) != 0) sleep(1);
    while (setreuid(1536, 1536) != 0) sleep(1);

    struct rlimit LIM; // time limit, file limit& memory limit
    LIM.rlim_cur = (solution_info->time_lmt / solution_info->cpu_compensation / 1000.00) + 1;
    LIM.rlim_max = LIM.rlim_cur;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(0);
    alarm(solution_info->time_lmt * 5.0 / solution_info->cpu_compensation);

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

    if (LANG_C == solution_info->lang_id
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
            execl("./Main", "./Main", NULL);
            break;
        case LANG_JAVA:
            execl(env->JAVA, env->JAVA, java_xms, java_xmx,
                  "-Djava.security.manager",
                  "-Djava.security.policy=./java.policy",
                  "Main", NULL);
            break;
        case LANG_RUBY:
            execl(env->RUBY, env->RUBY, "Main.rb", NULL);
            break;
        case LANG_BASH:
            execl("/bin/bash", "/bin/bash", "Main.sh", NULL);
            break;
        case LANG_PYTHON2:
            execl(env->PYTHON2, env->PYTHON2,
                  "Main.py",
                  NULL);
            break;
        case LANG_PHP:
            execl(env->PHP, env->PHP, "Main.php", NULL);
            break;
        case LANG_PERL:
            execl(env->PERL, env->PERL, "Main.pl", NULL);
            break;
        case LANG_C_SHARP:
            execl(env->MONO, env->MONO, "--debug", "Main.exe", NULL);
            break;
        case LANG_SCHEME:
            execl(env->GUILE, env->GUILE, "Main.scm", NULL);
            break;
        case LANG_SBCL:
            execl(env->SBCL, env->SBCL, "--script", "Main.cl", NULL);
            break;
        case LANG_LUA:
            execl(env->LUA, env->LUA, "Main", NULL);
            break;
        case LANG_JAVASCRIPT:
            execl(env->NODE, env->NODE, "Main.js", NULL);
            break;
        case LANG_PYTHON3:
            execl(env->PYTHON3, env->PYTHON3, "Main.py", NULL);
            break;
    }
    fflush(stderr);
    exit(0);
}

int special_judge(char *infile, char *outfile, char *user_file) {
    pid_t pid;
    pid = fork();
    int ret;
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

        ret = execute_cmd("%s/spj '%s' '%s' '%s'", solution_path, infile, outfile, user_file);
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

void judge_solution() {
    char *in_file = create_str(1024);
    char *out_file = create_str(1024);
    char *user_file = create_str(1024);

    sprintf(in_file, "%s/run/data.in", solution_path);
    sprintf(out_file, "%s/run/data.out", solution_path);
    sprintf(user_file, "%s/run/user.out", solution_path);


    if (solution_info->result_time > solution_info->time_lmt * 1000) {
        solution_info->result = OJ_TL;
        return;
    }
    if (solution_info->result_memory > solution_info->mem_lmt * STD_MB) {
        solution_info->result = OJ_ML;
        return;
    }

    if (solution_info->is_special_judge) {
        solution_info->result = special_judge(in_file, out_file, user_file) == 0 ? OJ_AC : OJ_WA;
    } else {
        solution_info->result = compare(out_file, user_file);
    }
}

bool compile() {
    char *compile_cmd[100];
    for (int i = 0; i < 100; ++i) {
        compile_cmd[i] = create_str(1024);
    }

    compile_cmd[LANG_C] = "gcc Main.c  -o Main -O2          -w -fmax-errors=10 -lm --static -std=c99   -DONLINE_JUDGE";
    compile_cmd[LANG_CXX] = "g++ Main.cc -o Main -O2 -fno-asm -w -fmax-errors=10 -lm --static -std=c++11 -DONLINE_JUDGE";
    sprintf(compile_cmd[LANG_PASCAL], "%s Main.pas -Cs32000000 -Sh -O2 -Co -Ct -Ci", env->FPC);
    sprintf(compile_cmd[LANG_JAVA], "%s -encoding UTF-8 Main.java", env->JAVAC);
    sprintf(compile_cmd[LANG_RUBY], "%s -c Main.rb", env->RUBY);
    compile_cmd[LANG_BASH] = "chmod +rx Main.sh";
    compile_cmd[LANG_PYTHON2] = "python2.7 -c import py_compile; py_compile.compile(r'Main.py')";
    compile_cmd[LANG_PYTHON3] = "python3.7 -c import py_compile; py_compile.compile(r'Main.py')";
    sprintf(compile_cmd[LANG_GO], "%s build -o Main Main.go", env->GO);
    sprintf(compile_cmd[LANG_SCHEME], "echo hello > /dev/null");
    compile_cmd[LANG_SBCL] = "echo hello > /dev/null";
    compile_cmd[LANG_CLANG] = "clang Main.c -o Main -ferror-limit=10 -fno-asm -Wall -lm --static -std=c99 -DONLINE_JUDGE";
    compile_cmd[LANG_CLANG_XX] = "clang++ Main.cc -o Main -ferror-limit=10 -fno-asm -Wall -lm --static -std=c++0x -DONLINE_JUDGE";
    compile_cmd[LANG_FREE_BASIC] = "fbc -lang qb Main.bas";
    sprintf(compile_cmd[LANG_C_SHARP], "%s -warn:0 Main.cs", env->MCS);
    sprintf(compile_cmd[LANG_LUA], "%s -o Main Main.lua", env->LUAC);
    sprintf(compile_cmd[LANG_JAVASCRIPT], "%s -c Main.js", env->NODE);
    compile_cmd[LANG_OBJC] = "gcc -o Main Main.m -fconstant-string-class=NSConstantString -I /usr/include/GNUstep/ -L /usr/lib/GNUstep/Libraries/ -lobjc -lgnustep-base";
    sprintf(compile_cmd[LANG_PHP], "%s -l Main.php", env->PHP);
    sprintf(compile_cmd[LANG_PERL], "%s -c Main.pl", env->PERL);

    int pid = fork();
    if (pid == 0) {
        chdir(run_dir);
        if (solution_info->lang_id != LANG_PASCAL && solution_info->lang_id != LANG_FREE_BASIC) {
            freopen("ce.txt", "w", stderr);
            freopen("/dev/null", "w", stdout);
        } else {
            freopen("ce.txt", "w", stdout);
            freopen("/dev/null", "w", stderr);
        }

        execute_cmd(compile_cmd[solution_info->lang_id]);
        exit(0);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        char file_path[1024];
        sprintf(file_path, "%s/run/ce.txt", solution_path);
        size_t size = get_file_size(file_path);
        if (size > 4) {
            return false;
        }
        return true;
    }
}

/**
 * main() function
 *
 * Usage: justoj-core-client -d <solution path>
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {
    if (argv_switch_exist(argc, argv, "--version")) {
        printf("Version: %s\n", get_version());
        return EXIT_SUCCESS;
    }

    /* 0. 参数判断，是否有 -d 参数 */
    if (argv_switch_not_exist(argc, argv, "-d") || argv_switch_exist(argc, argv, "--help")) {
        printf("Usage: justoj-core-client -d <solution path>");
        return EXIT_FAILURE;
    }
    solution_path = get_argv(argc, argv, "-d");
    if (strcmp(solution_path, "") == 0) {
        printf("ERROR: solution_path is empty");
        return EXIT_FAILURE;
    }

    /* 1. 创建环境信息 */
    init_data();

    /* 2. 编译代码 */
    if (compile() == false) {
        // 编译失败则直接退出
        return EXIT_SUCCESS;
    }


    /* 3. 检查目录是否存在 */
    DIR *dp;
    if ((dp = opendir(run_data_dir)) == NULL) {
        // 目录不存在，运行错误
        return EXIT_SUCCESS;
    }

    /* 4. 运行所有测试数据 */
    char *result_file_path = create_str(1024);
    sprintf(result_file_path, "%s/run/results.txt", solution_path);
    FILE *result_file = fopen(result_file_path, "w");
    fprintf(result_file, "[");
    struct dirent *dir_p;
    bool first_result = true;
    while ((dir_p = readdir(dp)) != NULL) {
        char *file_name = get_in_file_name(dir_p->d_name);
        if (strcmp(file_name, "") == 0) {
            continue;
        }

        execute_cmd("cp    %s/run/data/%s.in     %s/run/data.in", solution_path, file_name, solution_path);
        execute_cmd("cp    %s/run/data/%s.out    %s/run/data.out", solution_path, file_name, solution_path);
        init_syscall_limits(solution_info);
        pid_t pid = fork();
        if (pid == 0) {
            run_solution();
        } else {
            watch_solution(solution_info, pid);
            judge_solution();

            // 记录运行结果
            if (!first_result) {
                fprintf(result_file, ",\n");
            }
            first_result = false;
            execute_cmd("echo test >> %s/run/results.txt", solution_path);
            fprintf(result_file, "{\"name\": \"%s\", \"result\": %d, \"time\": %d, \"mem\": %d}", file_name,
                    solution_info->result, solution_info->result_time, solution_info->result_memory);
        }
    }
    fprintf(result_file, "]");

    return EXIT_SUCCESS;
}
