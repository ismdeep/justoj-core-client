//
// Created by Del Cooper on 2020/7/4.
//

#ifndef JUSTOJ_CORE_CLIENT_DEFINES_H
#define JUSTOJ_CORE_CLIENT_DEFINES_H

#define LANG_C 0
#define LANG_CXX 1
#define LANG_PASCAL 2
#define LANG_JAVA 3
#define LANG_RUBY 4
#define LANG_BASH 5
#define LANG_PYTHON2 6
#define LANG_PHP 7
#define LANG_PERL 8
#define LANG_C_SHARP 9
#define LANG_OBJC 10
#define LANG_FREE_BASIC 11
#define LANG_SCHEME 12
#define LANG_CLANG 13
#define LANG_CLANG_XX 14
#define LANG_LUA 15
#define LANG_JAVASCRIPT 16
#define LANG_GO 17
#define LANG_PYTHON3 18
#define LANG_SBCL 19

#define OJ_WT0 0
#define OJ_WT1 1
#define OJ_CI 2
#define OJ_RI 3
#define OJ_AC 4
#define OJ_PE 5
#define OJ_WA 6
#define OJ_TL 7
#define OJ_ML 8
#define OJ_OL 9
#define OJ_RE 10
#define OJ_CE 11
#define OJ_CO 12
#define OJ_TR 13

static char lang_ext[20][8] = {
        "c", "cc", "pas", "java", "rb",
        "sh", "py", "php", "pl", "cs",
        "m", "bas", "scm", "c", "cc",
        "lua", "js", "go", "py", "cl"};

#define STD_MB 1048576LL
#define STD_T_LIM 2
#define STD_F_LIM (STD_MB<<5)  //default file size limit 32m ,2^5=32
#define STD_M_LIM (STD_MB<<7)  //default memory limit 128m ,2^7=128

#define CALL_ARRAY_SIZE 512

#define REG_SYSCALL orig_rax
#define REG_RET rax
#define REG_ARG0 rdi
#define REG_ARG1 rsi


#endif //JUSTOJ_CORE_CLIENT_DEFINES_H
