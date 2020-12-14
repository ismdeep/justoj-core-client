//
// Created by ismdeep on 2019/12/18.
//

#ifndef JUSTOJ_CORE_UTIL_H
#define JUSTOJ_CORE_UTIL_H

#include <ctype.h>
#include <stdarg.h>

#include <log.h>

#include <defines.h>
#include <sys/stat.h>

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


void read_size_t(char *buf, const char *key, size_t *value) {
    char buf2[BUFFER_SIZE];
    if (read_buf(buf, key, buf2))
        sscanf(buf2, "%zu", value);
}

void read_uint64(char *buf, const char *key, uint64_t *value) {
    char buf2[BUFFER_SIZE];
    if (read_buf(buf, key, buf2)) {
        sscanf(buf2, "%lu", value);
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

const char *getFileNameFromPath(const char *path) {
    for (int i = strlen(path); i >= 0; i--) {
        if (path[i] == '/')
            return &path[i + 1];
    }
    return path;
}

void find_next_nonspace(int *c1, int *c2, FILE *f1, FILE *f2, int *ret) {
    // Find the next non-space character or \n.
    while ((isspace(*c1)) || (isspace(*c2))) {
        if (*c1 != *c2) {
            if (*c2 == EOF) {
                do {
                    *c1 = fgetc(f1);
                } while (isspace(*c1));
                continue;
            } else if (*c1 == EOF) {
                do {
                    *c2 = fgetc(f2);
                } while (isspace(*c2));
                continue;
            } else if (isspace(*c1) && isspace(*c2)) {
                while (*c2 == '\n' && isspace(*c1) && *c1 != '\n') *c1 = fgetc(f1);
                while (*c1 == '\n' && isspace(*c2) && *c2 != '\n') *c2 = fgetc(f2);
            } else {
                *ret = OJ_AC;
            }
        }
        if (isspace(*c1)) {
            *c1 = fgetc(f1);
        }
        if (isspace(*c2)) {
            *c2 = fgetc(f2);
        }
    }
}

void delnextline(char s[]) {
    int L;
    L = strlen(s);
    while (L > 0 && (s[L - 1] == '\n' || s[L - 1] == '\r'))
        s[--L] = 0;
}

int compare(const char *file1, const char *file2) {
    int ret;
    ret = OJ_AC;
    int c1 = 0, c2 = 0;
    FILE *f1, *f2;
    f1 = fopen(file1, "re");
    f2 = fopen(file2, "re");
    if (!f1 || !f2) {
        ret = OJ_RE;
    } else
        while (true) {
            // Find the first non-space character at the beginning of line.
            // Blank lines are skipped.
            c1 = fgetc(f1);
            c2 = fgetc(f2);
            find_next_nonspace(&c1, &c2, f1, f2, &ret);
            // Compare the current line.
            while (true) {
                // Read until 2 files return a space or 0 together.
                while ((!isspace(c1) && c1) || (!isspace(c2) && c2)) {
                    if (c1 == EOF && c2 == EOF) {
                        goto end;
                    }
                    if (c1 == EOF || c2 == EOF) {
                        break;
                    }
                    if (c1 != c2) {
                        // Consecutive non-space characters should be all exactly the same
                        ret = OJ_WA;
                        goto end;
                    }
                    c1 = fgetc(f1);
                    c2 = fgetc(f2);
                }
                find_next_nonspace(&c1, &c2, f1, f2, &ret);
                if (c1 == EOF && c2 == EOF) {
                    goto end;
                }
                if (c1 == EOF || c2 == EOF) {
                    ret = OJ_WA;
                    goto end;
                }

                if ((c1 == '\n' || !c1) && (c2 == '\n' || !c2)) {
                    break;
                }
            }
        }
    end:
    if (f1)
        fclose(f1);
    if (f2)
        fclose(f2);
    return ret;
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
