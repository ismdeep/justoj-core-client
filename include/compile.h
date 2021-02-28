//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2020/12/25.
//

#ifndef JUSTOJ_CORE_COMPILE_H
#define JUSTOJ_CORE_COMPILE_H

#include <system_info.h>

#include <defines.h>

int compile(const struct SystemInfo *system_info, int lang) {
    int pid;
    const char *CP_C[] = {"gcc", "Main.c", "-o", "Main", "-O2", "-fmax-errors=10", "-Wall", "-lm", "--static",
                          "-std=c99", "-DONLINE_JUDGE", NULL};
    const char *CP_CXX[] = {"g++", "-fno-asm", "-O2", "-fmax-errors=10", "-Wall", "-lm", "--static", "-std=c++11",
                            "-DONLINE_JUDGE", "-o", "Main", "Main.cc", NULL};
    const char *CP_PASCAL[] = {"fpc", "Main.pas", "-Cs32000000", "-Sh", "-O2", "-Co", "-Ct", "-Ci", NULL};
    const char *CP_RUBY[] = {"ruby", "-c", "Main.rb", NULL};
    const char *CP_BASH[] = {"chmod", "+rx", "Main.sh", NULL};
    const char *CP_PYTHON2[] = {system_info->python2_path, "-c", "import py_compile; py_compile.compile(r'Main.py')", NULL};
    const char *CP_PYTHON3[] = {system_info->python3_path, "-c", "import py_compile; py_compile.compile(r'Main.py')", NULL};
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

    sprintf(CP_JAVA[0], "/usr/bin/javac");
    sprintf(CP_JAVA[1], "-J%s", system_info->java_xms);
    sprintf(CP_JAVA[2], "-J%s", system_info->java_xmx);
    sprintf(CP_JAVA[3], "-encoding");
    sprintf(CP_JAVA[4], "UTF-8");
    sprintf(CP_JAVA[5], "Main.java");
    CP_JAVA[6] = (char *) NULL;

    pid = fork();
    if (pid == 0) {
        struct rlimit LIM;
        int cpu = 20;
        if (lang == 3) cpu = 30;
        LIM.rlim_max = cpu;
        LIM.rlim_cur = cpu;
        setrlimit(RLIMIT_CPU, &LIM);
        alarm(cpu);
        LIM.rlim_max = 100 * STD_MB;
        LIM.rlim_cur = 100 * STD_MB;
        setrlimit(RLIMIT_FSIZE, &LIM);

        if (lang == 3 || lang == 17) {
            LIM.rlim_max = STD_MB << 12;
            LIM.rlim_cur = STD_MB << 12;
        } else {
            LIM.rlim_max = STD_MB << 11;
            LIM.rlim_cur = STD_MB << 11;
        }
        if (lang != 3) {
            setrlimit(RLIMIT_AS, &LIM);
        }
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

#endif //JUSTOJ_CORE_COMPILE_H
