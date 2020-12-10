//
// Created by ismdeep on 2019/12/18.
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

#include <defines.h>

#include <log.h>

#define STD_MB 1048576LL
#define STD_T_LIM 2
#define STD_F_LIM (STD_MB<<5)  //default file size limit 32m ,2^5=32
#define STD_M_LIM (STD_MB<<7)  //default memory limit 128m ,2^7=128

#define CALL_ARRAY_SIZE 512

#define REG_SYSCALL orig_rax
#define REG_RET rax
#define REG_ARG0 rdi
#define REG_ARG1 rsi

static char data_list[BUFFER_SIZE][BUFFER_SIZE];
static int data_list_len = 0;

static int java_time_bonus = 5;
static int java_memory_bonus = 512;
static char java_xms[BUFFER_SIZE];
static char java_xmx[BUFFER_SIZE];
static int full_diff = 0;
static int use_max_time = 0;

static char client_name[BUFFER_SIZE];
static char http_base_url[BUFFER_SIZE];
static char secure_code[BUFFER_SIZE];

static double cpu_compensation = 1.0;
static char record_call = 0;
static int use_ptrace = 1;

char *python2;
char *python3;
char *guile;
char *scheme;
char *sbcl;
char *nodejs;

unsigned int call_id = 0;
unsigned int call_counter[CALL_ARRAY_SIZE] = {0};
static char LANG_NAME[BUFFER_SIZE];

void init_syscall_limits(int lang) {
    int i;
    memset(call_counter, 0, sizeof(call_counter));

    switch (lang) {
        case LANG_C:
        case LANG_CXX:
        case LANG_CLANG:
        case LANG_CLANG_XX:
            for (i = 0; i == 0 || LANG_CV[i]; i++) call_counter[LANG_CV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PASCAL:
            for (i = 0; i == 0 || LANG_PV[i]; i++) call_counter[LANG_PV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_JAVA:
            for (i = 0; i == 0 || LANG_JV[i]; i++) call_counter[LANG_JV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_RUBY:
            for (i = 0; i == 0 || LANG_RV[i]; i++) call_counter[LANG_RV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_BASH:
            for (i = 0; i == 0 || LANG_BV[i]; i++) call_counter[LANG_BV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PYTHON2:
            for (i = 0; i == 0 || LANG_YV[i]; i++) call_counter[LANG_YV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PHP:
            for (i = 0; i == 0 || LANG_PHV[i]; i++) call_counter[LANG_PHV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PERL:
            for (i = 0; i == 0 || LANG_PLV[i]; i++) call_counter[LANG_PLV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_C_SHARP:
            for (i = 0; i == 0 || LANG_CSV[i]; i++) call_counter[LANG_CSV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_OBJC:
            for (i = 0; i == 0 || LANG_OV[i]; i++) call_counter[LANG_OV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_FREE_BASIC:
            for (i = 0; i == 0 || LANG_BASICV[i]; i++) call_counter[LANG_BASICV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_SCHEME:
            for (i = 0; i == 0 || LANG_SV[i]; i++) call_counter[LANG_SV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_SBCL:
            for (i = 0; i == 0 || LANG_SBCLV[i]; i++) call_counter[LANG_SBCLV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_LUA:
            for (i = 0; i == 0 || LANG_LUAV[i]; i++) call_counter[LANG_LUAV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_JAVASCRIPT:
            for (i = 0; i == 0 || LANG_JSV[i]; i++) call_counter[LANG_JSV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_GO:
            for (i = 0; i == 0 || LANG_GOV[i]; i++) call_counter[LANG_GOV[i]] = HOJ_MAX_LIMIT;
            break;
        case LANG_PYTHON3:
            for (i = 0; i == 0 || LANG_YV[i]; i++) call_counter[LANG_YV[i]] = HOJ_MAX_LIMIT;
            break;
    }
}


/* Read Judge config */
void init_judge_conf(char *oj_home) {
    python2 = malloc(sizeof(char) * 65535);
    python3 = malloc(sizeof(char) * 65535);
    guile = malloc(sizeof(char) * 65535);
    scheme = malloc(sizeof(char) * 65535);
    sbcl = malloc(sizeof(char) * 65535);
    nodejs = malloc(sizeof(char) * 65535);

    FILE *fp = NULL;
    char config_file_path[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    strcpy(java_xms, "-Xms32m");
    strcpy(java_xmx, "-Xmx256m");
    sprintf(config_file_path, "%s/etc/judge.conf", oj_home);
    fp = fopen(config_file_path, "re");
    if (fp != NULL) {
        while (fgets(buf, BUFFER_SIZE - 1, fp)) {
            read_int(buf, "OJ_JAVA_TIME_BONUS", &java_time_bonus);
            read_int(buf, "OJ_JAVA_MEMORY_BONUS", &java_memory_bonus);
            read_buf(buf, "OJ_JAVA_XMS", java_xms);
            read_buf(buf, "OJ_JAVA_XMX", java_xmx);
            read_buf(buf, "OJ_HTTP_BASE_URL", http_base_url);
            read_buf(buf, "OJ_SECURE_CODE", secure_code);
            read_buf(buf, "OJ_CLIENT_NAME", client_name);
            read_int(buf, "OJ_FULL_DIFF", &full_diff);
            read_int(buf, "OJ_USE_MAX_TIME", &use_max_time);
            read_int(buf, "OJ_USE_PTRACE", &use_ptrace);
            read_double(buf, "OJ_CPU_COMPENSATION", &cpu_compensation);
            read_buf(buf, "PYTHON2", python2);
            read_buf(buf, "PYTHON3", python3);
            read_buf(buf, "GUILE", guile);
            read_buf(buf, "SCHEME", scheme);
            read_buf(buf, "SBCL", sbcl);
            read_buf(buf, "NODEJS", nodejs);
        }
        fclose(fp);
    } else {
        log_info("FILE NOT FOUND. [%s]", config_file_path);
        exit(-1);
    }
}

void update_solution(int solution_id, int result, int time, int memory) {
    if (result == OJ_TL && memory == 0)
        result = OJ_ML;
    log_info("==> [%s] %d %s", client_name, solution_id, result_text[result]);
    judge_http_api_update_solution(http_base_url, secure_code, solution_id, result, time, memory);
}

int get_proc_status(int pid, const char *mark) {
    FILE *pf;
    char fn[BUFFER_SIZE], buf[BUFFER_SIZE];
    int ret = 0;
    sprintf(fn, "/proc/%d/status", pid);
    pf = fopen(fn, "re");
    int m = strlen(mark);
    while (pf && fgets(buf, BUFFER_SIZE - 1, pf)) {

        buf[strlen(buf) - 1] = 0;
        if (strncmp(buf, mark, m) == 0) {
            sscanf(buf + m + 1, "%d", &ret);
        }
    }
    if (pf)
        fclose(pf);
    return ret;
}


void get_solution(int solution_id, char *work_dir, int lang) {
    char src_pth[BUFFER_SIZE];
    sprintf(src_pth, "Main.%s", lang_ext[lang]);
    judge_http_api_get_solution(http_base_url, secure_code, solution_id, lang);
    execute_cmd("chown judge %s/%s", work_dir, src_pth);
}


void get_solution_info(int solution_id, int *p_id, char *user_id, int *lang) {
    judge_http_api_get_solution_info(http_base_url, secure_code, solution_id, p_id, user_id, lang);
}


void get_problem_info(int p_id, int *time_lmt, int *mem_lmt, int *isspj) {
    _get_problem_info_http(http_base_url, secure_code, p_id, time_lmt, mem_lmt, isspj);
    if (*time_lmt <= 0)
        *time_lmt = 1;
}

int compile(int lang, char *work_dir) {
    int pid;
    const char *CP_C[] = {"gcc", "Main.c", "-o", "Main", "-O2", "-fmax-errors=10", "-Wall", "-lm", "--static",
                          "-std=c99", "-DONLINE_JUDGE", NULL};
    const char *CP_CXX[] = {"g++", "-fno-asm", "-O2", "-fmax-errors=10", "-Wall", "-lm", "--static", "-std=c++11",
                            "-DONLINE_JUDGE", "-o", "Main", "Main.cc", NULL};
    const char *CP_PASCAL[] = {"fpc", "Main.pas", "-Cs32000000", "-Sh", "-O2", "-Co", "-Ct", "-Ci", NULL};
    const char *CP_RUBY[] = {"ruby", "-c", "Main.rb", NULL};
    const char *CP_BASH[] = {"chmod", "+rx", "Main.sh", NULL};
    const char *CP_PYTHON2[] = {python2, "-c", "import py_compile; py_compile.compile(r'Main.py')", NULL};
    const char *CP_PYTHON3[] = {python3, "-c", "import py_compile; py_compile.compile(r'Main.py')", NULL};
    const char *CP_PHP[] = {"php", "-l", "Main.php", NULL};
    const char *CP_PERL[] = {"perl", "-c", "Main.pl", NULL};
    const char *CP_C_SHARP[] = {"gmcs", "-warn:0", "Main.cs", NULL};
    const char *CP_OBJ_C[] = {"gcc", "-o", "Main", "Main.m",
                              "-fconstant-string-class=NSConstantString", "-I",
                              "/usr/include/GNUstep/", "-L", "/usr/lib/GNUstep/Libraries/",
                              "-lobjc", "-lgnustep-base", NULL};
    const char *CP_FREE_BASIC[] = {"fbc", "-lang", "qb", "Main.bas", NULL};
    const char *CP_CLANG[] = {"clang", "Main.c", "-o", "Main", "-ferror-limit=10", "-fno-asm", "-Wall",
                              "-lm", "--static", "-std=c99", "-DONLINE_JUDGE", NULL};
    const char *CP_CLANG_CXX[] = {"clang++", "Main.cc", "-o", "Main", "-ferror-limit=10", "-fno-asm", "-Wall",
                                  "-lm", "--static", "-std=c++0x", "-DONLINE_JUDGE", NULL};
    const char *CP_LUA[] = {"luac", "-o", "Main", "Main.lua", NULL};
    const char *CP_JS[] = {"js24", "-c", "Main.js", NULL};
    const char *CP_GO[] = {"go", "build", "-o", "Main", "Main.go", NULL};

    char javac_buf[7][32];
    char *CP_JAVA[7];

    for (int i = 0; i < 7; i++)
        CP_JAVA[i] = javac_buf[i];

    sprintf(CP_JAVA[0], "javac");
    sprintf(CP_JAVA[1], "-J%s", java_xms);
    sprintf(CP_JAVA[2], "-J%s", java_xmx);
    sprintf(CP_JAVA[3], "-encoding");
    sprintf(CP_JAVA[4], "UTF-8");
    sprintf(CP_JAVA[5], "Main.java");
    CP_JAVA[6] = (char *) NULL;

    pid = fork();
    if (pid == 0) {
        struct rlimit LIM;
        int cpu = 6;
        if (lang == 3) cpu = 30;
        LIM.rlim_max = cpu;
        LIM.rlim_cur = cpu;
        setrlimit(RLIMIT_CPU, &LIM);
        alarm(cpu);
        LIM.rlim_max = 40 * STD_MB;
        LIM.rlim_cur = 40 * STD_MB;
        setrlimit(RLIMIT_FSIZE, &LIM);

        if (lang == 3 || lang == 17) {
            LIM.rlim_max = STD_MB << 12;
            LIM.rlim_cur = STD_MB << 12;
        } else {
            LIM.rlim_max = STD_MB * 512;
            LIM.rlim_cur = STD_MB * 512;
        }
        if (lang != 3)setrlimit(RLIMIT_AS, &LIM);
        if (lang != 2 && lang != 11) {
            freopen("ce.txt", "w", stderr);
            //freopen("/dev/null", "w", stdout);
        } else {
            freopen("ce.txt", "w", stdout);
        }

        while (setgid(1536) != 0) sleep(1);
        while (setuid(1536) != 0) sleep(1);

        while (setreuid(1536, 1536) != 0) sleep(1);

        switch (lang) {
            case LANG_C:
                execvp(CP_C[0], (char *const *) CP_C);
                break;
            case LANG_CXX:
                execvp(CP_CXX[0], (char *const *) CP_CXX);
                break;
            case LANG_PASCAL:
                execvp(CP_PASCAL[0], (char *const *) CP_PASCAL);
                break;
            case LANG_JAVA:
                execvp(CP_JAVA[0], (char *const *) CP_JAVA);
                break;
            case LANG_RUBY:
                execvp(CP_RUBY[0], (char *const *) CP_RUBY);
                break;
            case LANG_BASH:
                execvp(CP_BASH[0], (char *const *) CP_BASH);
                break;
            case LANG_PYTHON2:
                execvp(CP_PYTHON2[0], (char *const *) CP_PYTHON2);
                break;
            case LANG_PYTHON3:
                execvp(CP_PYTHON3[0], (char *const *) CP_PYTHON3);
                break;
            case LANG_PHP:
                execvp(CP_PHP[0], (char *const *) CP_PHP);
                break;
            case LANG_PERL:
                execvp(CP_PERL[0], (char *const *) CP_PERL);
                break;
            case LANG_C_SHARP:
                execvp(CP_C_SHARP[0], (char *const *) CP_C_SHARP);
                break;
            case LANG_OBJC:
                execvp(CP_OBJ_C[0], (char *const *) CP_OBJ_C);
                break;
            case LANG_FREE_BASIC:
                execvp(CP_FREE_BASIC[0], (char *const *) CP_FREE_BASIC);
                break;
            case LANG_SCHEME:
                break;
            case LANG_CLANG:
                execvp(CP_CLANG[0], (char *const *) CP_CLANG);
                break;
            case LANG_CLANG_XX:
                execvp(CP_CLANG_CXX[0], (char *const *) CP_CLANG_CXX);
                break;
            case LANG_LUA:
                execvp(CP_LUA[0], (char *const *) CP_LUA);
                break;
            case LANG_JAVASCRIPT:
                execvp(CP_JS[0], (char *const *) CP_JS);
                break;
            case LANG_GO:
                execvp(CP_GO[0], (char *const *) CP_GO);
                break;
            case LANG_SBCL:
                break;
            default:
                log_info("nothing to do!\n");
        }
        exit(0);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        if (lang > 3 && lang < 7) {
            status = get_file_size("ce.txt");
        }
        return status;
    }
}

void prepare_files(
        char *oj_home, char *filename, int namelen, char *infile, const int *p_id,
        char *work_dir, char *outfile, char *userfile, int solution_id) {
    char fname0[BUFFER_SIZE];
    char fname[BUFFER_SIZE];
    strncpy(fname0, filename, namelen);
    fname0[namelen] = 0;
    escape(fname, fname0);
    sprintf(infile, "%s/data/%d/%s.in", oj_home, *p_id, fname);
    execute_cmd("/bin/cp '%s' %s/data.in", infile, work_dir);
    execute_cmd("/bin/cp %s/data/%d/*.dic %s/ 2> /dev/null", oj_home, *p_id, work_dir);

    sprintf(outfile, "%s/data/%d/%s.out", oj_home, *p_id, fname0);
    sprintf(userfile, "%s/run%d/user.out", oj_home, solution_id);
}

void run_solution(int lang, char *work_dir, int *time_lmt, int *usedtime, int *mem_lmt) {
    nice(19);
    // now the user is "judger"
    chdir(work_dir);
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
    LIM.rlim_cur = (*time_lmt / cpu_compensation - *usedtime / 1000) + 1;
    LIM.rlim_max = LIM.rlim_cur;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(0);
    alarm(*time_lmt * 5 / cpu_compensation);

    // file limit
    LIM.rlim_max = STD_F_LIM + STD_MB;
    LIM.rlim_cur = STD_F_LIM;
    setrlimit(RLIMIT_FSIZE, &LIM);
    // proc limit
    switch (lang) {
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
    LIM.
            rlim_cur = STD_MB * *mem_lmt / 2 * 3;
    LIM.
            rlim_max = STD_MB * *mem_lmt * 2;
    if (
            LANG_C == lang || LANG_CXX == lang || LANG_PASCAL == lang
            || LANG_OBJC == lang || LANG_CLANG == lang || LANG_CLANG_XX == lang) {
        setrlimit(RLIMIT_AS, &LIM);
    }


    switch (lang) {
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
            sprintf(java_xms, "-Xmx%dM", *mem_lmt);
            execl("/usr/bin/java", "/usr/bin/java", java_xms, java_xmx,
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
            execl(python2, python2, "Main.py",
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
            execl(guile, guile, "Main.scm", (char *) NULL);
            break;
        case LANG_SBCL:
            execl(sbcl, sbcl, "--script", "Main.cl", (char *) NULL);
            break;
        case LANG_LUA:
            execl("/lua", "/lua", "Main", (char *) NULL);
            break;
        case LANG_JAVASCRIPT:
            execl("/nodejs", "/nodejs", "Main.js", (char *) NULL);
            break;
        case LANG_PYTHON3:
            execl(python3, python3, "Main.py", (char *) NULL);
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

int special_judge(char *oj_home, int problem_id, char *infile, char *outfile,
                  char *userfile) {

    pid_t pid;
    log_info("pid=%d\n", problem_id);
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

        ret = execute_cmd("%s/data/%d/spj '%s' '%s' %s", oj_home, problem_id,
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

void judge_solution(char *oj_home, int *ACflg, int *usedtime, int time_lmt, int isspj, int p_id,
                    char *infile, char *outfile, char *userfile,
                    int lang, char *work_dir, int *topmemory, int mem_lmt,
                    int num_of_test) {
//usedtime-=1000;
    int comp_res;
    num_of_test = 1;
    if (*ACflg == OJ_AC
        && *usedtime > time_lmt * 1000 * (use_max_time ? 1 : num_of_test))
        *ACflg = OJ_TL;
    if (*topmemory > mem_lmt * STD_MB)
        *ACflg = OJ_ML; //issues79
// compare
    if (*ACflg == OJ_AC) {
        if (isspj) {
            comp_res = special_judge(oj_home, p_id, infile, outfile, userfile);

            if (comp_res == 0)
                comp_res = OJ_AC;
            else {
                comp_res = OJ_WA;
            }
        } else {
            comp_res = compare(outfile, userfile);
        }
        if (comp_res == OJ_WA) {
            *ACflg = OJ_WA;
        }
        *ACflg = comp_res;
    }
//jvm popup messages, if don't consider them will get miss-WrongAnswer
    if (lang == 3) {
        comp_res = fix_java_mis_judge(work_dir, ACflg, topmemory, mem_lmt);
    }
    if (lang == 6) {
        comp_res = fix_python_mis_judge(work_dir, ACflg, topmemory, mem_lmt);
    }
}

int get_page_fault_mem(struct rusage *ruse, pid_t *pidApp) {
    /* java use pagefault */
    int m_vmpeak, m_vmdata, m_minflt;
    m_minflt = ruse->ru_minflt * getpagesize();
    m_vmpeak = get_proc_status(*pidApp, "VmPeak:");
    m_vmdata = get_proc_status(*pidApp, "VmData:");
    // printf("VmPeak:%d KB VmData:%d KB minflt:%d KB\n", m_vmpeak, m_vmdata, m_minflt >> 10);
    return m_minflt;
}

void print_runtimeerror(char *err) {
    log_info("Runtime Error: %s", err);
}

void watch_solution(
        pid_t pidApp,
        char *infile,
        int *ACflg,
        int isspj,
        char *userfile,
        char *outfile,
        int solution_id,
        int lang,
        int *topmemory,
        int mem_lmt,
        int *usedtime,
        int time_lmt
) {
    /* parent */
    int tempmemory = 0;

    int status, sig, exitcode;
    struct user_regs_struct reg;
    struct rusage ruse;
    int first = true;
    while (1) {
        /* check the usage */
        wait4(pidApp, &status, __WALL, &ruse);
        if (first) { //
            ptrace(PTRACE_SETOPTIONS, pidApp, NULL, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT);
        }

        //jvm gc ask VM before need,so used kernel page fault times and page size
        if (lang == 3 || lang == 7 || lang == 9 || lang == 13 || lang == 14 || lang == 16 || lang == 17) {
            tempmemory = get_page_fault_mem(&ruse, &pidApp);
        } else {        //other use VmPeak
            tempmemory = get_proc_status(pidApp, "VmPeak:") << 10;
        }
        if (tempmemory > *topmemory)
            *topmemory = tempmemory;
        if (*topmemory > mem_lmt * STD_MB) {
            if (*ACflg == OJ_AC)
                *ACflg = OJ_ML;
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }
        /* sig = status >> 8; status >> 8 EXITCODE */
        if (WIFEXITED(status))
            break;
        if ((lang < 4 || lang == 9) && get_file_size("error.out")) {
            *ACflg = OJ_RE;
            /* addreinfo(solution_id); */
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }

        if (!isspj && get_file_size(userfile) > get_file_size(outfile) * 2 + 1024) {
            *ACflg = OJ_OL;
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }

        exitcode = WEXITSTATUS(status);
/*exitcode == 5 waiting for next CPU allocation          * ruby using system to run,exit 17 ok
 *  Runtime Error:Unknown signal xxx need be added here
 */
        if ((lang >= 3 && exitcode == 17) || exitcode == 0x05 || exitcode == 0 || exitcode == 133) {
            //go on and on
            ;
        } else {
            //psignal(exitcode, NULL);
            if (*ACflg == OJ_AC) {
                switch (exitcode) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    case SIGKILL:
                    case SIGXCPU:
                        *ACflg = OJ_TL;
                        *usedtime = time_lmt * 1000;
                        break;
                    case SIGXFSZ:
                        *ACflg = OJ_OL;
                        break;
                    default:
                        *ACflg = OJ_RE;
                }

                print_runtimeerror(strsignal(exitcode));
            }
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);

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

            if (*ACflg == OJ_AC) {
                switch (sig) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    case SIGKILL:
                    case SIGXCPU:
                        *ACflg = OJ_TL;
                        break;
                    case SIGXFSZ:
                        *ACflg = OJ_OL;
                        break;
                    default:
                        *ACflg = OJ_RE;
                }

                print_runtimeerror(strsignal(sig));

            }
            break;
        }
/*     comment from http://www.felix021.com/blog/read.php?1662

WIFSTOPPED: return true if the process is paused or stopped while ptrace is watching on it
WSTOPSIG: get the signal if it was stopped by signal
 */

// check the system calls
        ptrace(PTRACE_GETREGS, pidApp, NULL, &reg);
        call_id = (unsigned int) reg.REG_SYSCALL % CALL_ARRAY_SIZE;
        if (call_counter[call_id]) {
//call_counter[reg.REG_SYSCALL]--;
        } else if (record_call) {
            call_counter[call_id] = 1;

        } else { //do not limit JVM syscall for using different JVM
            *ACflg = OJ_RE;
            char error[BUFFER_SIZE];
            sprintf(error,
                    "[ERROR] A Not allowed system call: runid:%d CALLID:%u\n"
                    " TO FIX THIS , ask admin to add the CALLID into corresponding LANG_XXV[] located at okcalls32/64.h ,\n"
                    "and recompile justoj_core_client. \n"
                    "if you are admin and you don't know what to do ,\n"
                    "chinese explaination can be found on https://zhuanlan.zhihu.com/p/24498599\n",
                    solution_id, call_id);

            // printf("%s", error);
            print_runtimeerror(error);
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }


        ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);
        first = false;
    }
    *usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000) * cpu_compensation;
    *usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000) * cpu_compensation;
}

void init_parameters(int argc, const char **argv, char *oj_home, int *solution_id) {
    strcpy(oj_home, argv[1]);
    *solution_id = atoi(argv[2]);
}

void print_call_array() {
    log_info("int LANG_%sV[256]={", LANG_NAME);
    int i = 0;
    for (i = 0; i < CALL_ARRAY_SIZE; i++) {
        if (call_counter[i]) {
            log_info("%d,", i);
        }
    }
    log_info("0};");

    log_info("int LANG_%sC[256]={", LANG_NAME);
    for (i = 0; i < CALL_ARRAY_SIZE; i++) {
        if (call_counter[i]) {
            log_info("HOJ_MAX_LIMIT,");
        }
    }
    log_info("0};\n");
}

/**
 * main() function
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char **argv) {
    char work_dir[BUFFER_SIZE];
    static char base_path[BUFFER_SIZE];
    char user_id[BUFFER_SIZE];
    int solution_id;
    char run_work_dir[BUFFER_SIZE];
    int p_id, time_lmt, mem_lmt, lang, isspj, max_case_time = 0;

    /* 0. 判断命令行参数个数是否正确 */
    if (argc < 3) {
        fprintf(stderr, "Usage:%s ${base_path} ${solution_id}\n", argv[0]);
        exit(1);
    }

    /* 1. 读取命令行参数 */
    init_parameters(argc, argv, base_path, &solution_id);
    sprintf(work_dir, "%s/run%d", base_path, solution_id);
    sprintf(run_work_dir, "run%d", solution_id);

    char log_file_path[1024];
    sprintf(log_file_path, "%s/product.log", base_path);
    FILE *log_file = fopen(log_file_path, "ab");
    log_add_fp(log_file, LOG_INFO);
    log_set_quiet(false);

    /* 2. 读取配置文件 */
    init_judge_conf(base_path);
    log_info("==> [%s] %d START", client_name, solution_id);

    /* 3. 初始化环境 */
    chdir(base_path);
    execute_cmd("/bin/mkdir -p %s", run_work_dir);
    execute_cmd("chmod -R 777 run%d", solution_id);
    chdir(work_dir);

    /* 4. 获取Solution信息 */
    get_solution_info(solution_id, &p_id, user_id, &lang);

    /* 5. 获取题目信息 */
    get_problem_info(p_id, &time_lmt, &mem_lmt, &isspj);

    /* 6. 获取Solution源代码 */
    get_solution(solution_id, work_dir, lang);

    /* 7. 编译代码 */
    if (compile(lang, work_dir) != 0) {
        judge_http_api_add_ce_info_http(http_base_url, secure_code, solution_id);
        update_solution(solution_id, OJ_CE, 0, 0);
        /* 清理环境 */
        chdir(base_path);
        execute_cmd("rm -rf run%d", solution_id);
        exit(0);
    }

    /* 8. 配置时间超限、内存超限 */
    //java is lucky
    if (lang >= 3 && lang != 10 && lang != 13 && lang != 14) {  // Clang Clang++ not VM or Script
        // the limit for java
        time_lmt = time_lmt + java_time_bonus;
        mem_lmt = mem_lmt + java_memory_bonus;
        // copy java.policy
        if (lang == 3) {
            execute_cmd("/bin/cp %s/etc/java0.policy %s/java.policy 2> /dev/null", base_path, work_dir);
            execute_cmd("chmod 755 %s/java.policy 2> /dev/null", work_dir);
            execute_cmd("chown judge %s/java.policy 2> /dev/null", work_dir);
        }
    }
    if (time_lmt > 300 || time_lmt < 1)
        time_lmt = 300;
    if (mem_lmt > 1024 || mem_lmt < 1)
        mem_lmt = 1024;

    /* 9. 设置为正在运行状态 */
    update_solution(solution_id, OJ_RI, 0, 0);

    char *data_path = malloc(sizeof(char) * (strlen(base_path) + 255));
    char *in_file = malloc(sizeof(char) * (strlen(base_path) + 255));
    char *out_file = malloc(sizeof(char) * (strlen(base_path) + 255));
    char *user_file = malloc(sizeof(char) * (strlen(base_path) + 255));
    sprintf(data_path, "%s/data/%d", base_path, p_id);

    // open DIRs
    DIR *dp;
    struct dirent *dirp;

    // using http to get remote test data files
    if (p_id > 0 && (dp = opendir(data_path)) == NULL) {
        log_info("No such dir:%s!\n", data_path);
        exit(-1);
    }

    int ac_flag = OJ_AC;
    int name_len;
    int user_time = 0;
    int top_memory = 0;
    int num_of_test = 0;

    /* 10. 运行所有测试数据 */
    while (ac_flag == OJ_AC && (dirp = readdir(dp)) != NULL) {
        name_len = isInFile(dirp->d_name); // check if the file is *.in or not
        if (name_len == 0)
            continue;
        prepare_files(base_path, dirp->d_name, name_len, in_file, &p_id, work_dir, out_file,
                      user_file, solution_id);
        init_syscall_limits(lang);
        pid_t pidApp = fork();
        if (pidApp == 0) {
            run_solution(lang, work_dir, &time_lmt, &user_time, &mem_lmt);
        } else {
            num_of_test++;
            watch_solution(pidApp, in_file, &ac_flag, isspj, user_file, out_file,
                           solution_id, lang, &top_memory, mem_lmt, &user_time, time_lmt);
            judge_solution(base_path, &ac_flag, &user_time, time_lmt, isspj, p_id, in_file,
                           out_file, user_file, lang, work_dir, &top_memory,
                           mem_lmt, num_of_test);
            if (use_max_time) {
                max_case_time = user_time > max_case_time ? user_time : max_case_time;
                user_time = 0;
            }
        }
//        write_log("==> [%s] %d %s [%s] %dms", client_name, solution_id, result_text[ac_flag], dirp->d_name, user_time);
    }

    /* 11. 上传测试结果 */
    if (use_max_time) user_time = max_case_time;
    if (ac_flag == OJ_TL) user_time = time_lmt * 1000;
    update_solution(solution_id, ac_flag, user_time, top_memory >> 10);

    /* 清理环境 */
    chdir(base_path);
    execute_cmd("rm -rf run%d", solution_id);
    free(data_path);
    free(in_file);
    free(out_file);
    free(user_file);
    closedir(dp);

    return 0;
}
