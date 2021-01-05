//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2021/1/5.
//

#ifndef JUSTOJ_CORE_DATA_MONITOR_H
#define JUSTOJ_CORE_DATA_MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>
#include <ismdeep-c-utils/time.h>
#include <system_info.h>
#include <wget_utils.h>

char *malloc_str(size_t size) {
    char *str = (char *) malloc(sizeof(char) * size);
    memset(str, 0, sizeof(char) * size);
    return str;
}

void data_monitor(const struct SystemInfo *system_info) {
    char *cmd = malloc_str(1024);
    char *hash_info = malloc_str(1024);
    sprintf(cmd, "git -C %s log -1 --pretty=format:%%H", system_info->data_path);
    FILE *fp1 = wget_cmd_fp(cmd);
    fscanf(fp1, "%s", hash_info);
    fclose(fp1);

    char *url = malloc_str(1024);
    char *post_data = malloc_str(1024);

    sprintf(url, "%s/api/judge_client/push_data_info", system_info->http_base_url);
    sprintf(post_data, "secure_code=%s&client_name=%s&data_git_hash=%s",
            system_info->secure_code,
            system_info->client_name,
            hash_info);

    FILE *fp = wget_post_fp(url, post_data);
    fclose(fp);

    free(url);
    free(cmd);
    free(hash_info);
    free(post_data);
}

#endif