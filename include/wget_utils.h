//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2020/12/13.
//

#ifndef JUSTOJ_CORE_WGET_UTILS_H
#define JUSTOJ_CORE_WGET_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *wget_cmd_fp(char *cmd) {
    FILE *fp = popen(cmd, "r");
    return fp;
}


char *wget_cmd(char *cmd) {
    FILE *fp = wget_cmd_fp(cmd);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    char *content = (char *) malloc(sizeof(char) * (size + 1));
    memset(content, 0, sizeof(char) * (size + 1));
    rewind(fp);
    fread(content, sizeof(char), size, fp);
    content[size] = '\0';
    fclose(fp);
    return content;
}

FILE *wget_get_fp(char *url) {
    char *cmd = (char *) malloc(sizeof(char) * (strlen(url) + 255));
    sprintf(cmd, "wget -t 3 -q -O - \"%s\"", url);
    FILE *fp = wget_cmd_fp(cmd);
    return fp;
}

char *wget_get(char *url) {
    char *cmd = (char *) malloc(sizeof(char) * (strlen(url) + 255));
    sprintf(cmd, "wget -t 3 -q -O - \"%s\"", url);
    char *content = wget_cmd(cmd);
    free(cmd);
    return content;
}


FILE *wget_post_fp(char *url, char *post_data) {
    char *cmd = (char *) malloc(sizeof(char) * (strlen(url) + strlen(post_data) + 255));
    sprintf(cmd, "wget -t 3 --post-data=\"%s\" -q -O - \"%s\"", post_data, url);
    FILE *fp = wget_cmd_fp(cmd);
    free(cmd);
    return fp;
}

char *wget_post(char *url, char *post_data) {
    char *cmd = (char *) malloc(sizeof(char) * (strlen(url) + strlen(post_data) + 255));
    sprintf(cmd, "wget -t 3 --post-data=\"%s\" -q -O - \"%s\"", post_data, url);
    char *content = wget_cmd(cmd);
    free(cmd);
    return content;
}

#endif //JUSTOJ_CORE_WGET_UTILS_H
