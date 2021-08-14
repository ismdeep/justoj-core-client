//
// Created by ismdeep on 2019/12/18.
//

#ifndef JUSTOJ_CORE_UTIL_H
#define JUSTOJ_CORE_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <ctype.h>
#include <stdarg.h>

#include <log.h>

#include <defines.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 5120       //default size of char buffer 5120 bytes

int execute_cmd(const char *fmt, ...) {
    char cmd[BUFFER_SIZE];
    int ret = 0;
    va_list ap;
    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    ret = system(cmd);
    va_end(ap);
    return ret;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
    char *pstr = str, *buf = (char *) malloc(strlen(str) * 3 + 1), *pbuf = buf;
    while (*pstr) {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.'
            || *pstr == '~')
            *pbuf++ = *pstr;
        else if (*pstr == ' ')
            *pbuf++ = '+';
        else
            *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(
                    *pstr & 15);
        pstr++;
    }
    *pbuf = '\0';
    return buf;
}

// urlencoded function copied from http://www.geekhideout.com/urlcode.shtml
/* Converts a hex character to its integer value */
char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

int isInFile(const char fname[]) {
    int l = strlen(fname);
    if (l <= 3 || strcmp(fname + l - 3, ".in") != 0)
        return 0;
    else
        return l - 3;
}

#include <str_utils.h>

char * get_in_file_name(const char *file_name) {
    int l = strlen(file_name);
    if (l <= 3 || strcmp(file_name + l - 3, ".in") != 0) {
        return "";
    }

    char *result = create_str(1024);
    memset(result, 0, sizeof(char) * 1024);
    strcpy(result, file_name);
    result[l - 3] = '\0';
    return result;
}

int after_equal(const char *c) {
    int i = 0;
    for (; c[i] != '\0' && c[i] != '='; i++);
    return ++i;
}


void trim(char *c) {
    char buf[BUFFER_SIZE];
    char *start, *end;
    strcpy(buf, c);
    start = buf;
    while (isspace(*start))
        start++;
    end = start;
    while (!isspace(*end))
        end++;
    *end = '\0';
    strcpy(c, start);
}


bool read_buf(const char *buf, const char *key, char *value) {
    if (strncmp(buf, key, strlen(key)) == 0) {
        strcpy(value, buf + after_equal(buf));
        trim(value);
        return 1;
    }
    return 0;
}

void read_double(char *buf, const char *key, double *value) {
    char buf2[BUFFER_SIZE];
    if (read_buf(buf, key, buf2)) {
        sscanf(buf2, "%lf", value);
    }
}

void read_int(char *buf, const char *key, int *value) {
    char buf2[BUFFER_SIZE];
    if (read_buf(buf, key, buf2))
        sscanf(buf2, "%d", value);
}

int read_int_http(FILE *f) {
    char buf[BUFFER_SIZE];
    fgets(buf, BUFFER_SIZE - 1, f);
    return atoi(buf);
}

FILE *read_cmd_output(const char *fmt, ...) {
    char cmd[BUFFER_SIZE];

    FILE *ret = NULL;
    va_list ap;

    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    va_end(ap);
    ret = popen(cmd, "r");
    return ret;
}


unsigned char get_next_nonspace(FILE *fp) {
    unsigned char ch;
    while (true) {
        ch = fgetc(fp);
        if ((char)ch == ' ' || (char)ch == '\n' || (char)ch == '\t' || (char)ch == '\r') {
            continue;
        }
        return ch;
    }
}


int compare(const char *file1, const char *file2) {
    FILE *fp1 = fopen(file1, "re");
    FILE *fp2 = fopen(file2, "re");
    if (!fp1 || !fp2) {
        if (fp1) fclose(fp2);
        if (fp2) fclose(fp2);
        return OJ_RE;
    }

    unsigned char ch1, ch2;
    int result;

    while (true) {
        fflush(stdout);
        ch1 = get_next_nonspace(fp1);
        ch2 = get_next_nonspace(fp2);

        if (ch1 != ch2) {
            result = OJ_WA;
            break;
        }

        if ((char) ch1 == EOF) {
            result = OJ_AC;
            break;
        }
    }

    fclose(fp1);
    fclose(fp2);

    return result;
}

char *escape(char s[], const char t[]) {
    int i, j;
    for (i = j = 0; t[i] != '\0'; ++i) {
        if (t[i] == '\'') {
            s[j++] = '\'';
            s[j++] = '\\';
            s[j++] = '\'';
            s[j++] = '\'';
            continue;
        } else {
            s[j++] = t[i];
        }
    }
    s[j] = '\0';
    return s;
}

size_t get_file_size(const char *filename) {
    struct stat f_stat;
    if (stat(filename, &f_stat) == -1) {
        return 0;
    }
    return f_stat.st_size;
}

#endif //JUSTOJ_CORE_UTIL_H
