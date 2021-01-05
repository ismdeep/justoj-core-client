//
// Created by L. Jiang on 2020/12/25.
//

#ifndef JUSTOJ_CORE_SYSTEM_INFO_H
#define JUSTOJ_CORE_SYSTEM_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <str_utils.h>

struct SystemInfo {
    char *oj_home; // 判题机工作基础目录，包含 etc, data 文件夹
    char *client_name; // 判题机名称
    char *client_path; // 判题机路径
    char *data_path; // 数据路径到 data 文件夹, e.g. /justoj-data/data
    char *http_base_url; // 服务器网址
    char *secure_code; // 服务器访问安全密钥

    char *oj_lang_set; // 启用判题语言
    int max_running; // 同时运行判题数量
    int query_size; // 单次查询判题组大小
    int sleep_time; // 休眠时间，单位（s），默认为 1

    char *work_dir;

    char *python2_path;
    char *python3_path;
    char *guile_path;
    char *scheme_path;
    char *sbcl_path;
    char *nodejs_path;
    char *java_xms;
    char *java_xmx;
    double cpu_compensation;
};



struct SystemInfo *system_info_create() {
    struct SystemInfo *system_info = (struct SystemInfo *) malloc(sizeof(struct SystemInfo) * 1);

    system_info->oj_home = create_str(1024);
    system_info->client_name = create_str(1024);
    system_info->client_path = create_str(1024);
    system_info->data_path = create_str(1024);
    system_info->http_base_url = create_str(1024);
    system_info->secure_code = create_str(1024);
    system_info->oj_lang_set = create_str(1024);

    system_info->work_dir = create_str(1024);

    system_info->python2_path = create_str(1024);
    system_info->python3_path = create_str(1024);
    system_info->guile_path = create_str(1024);
    system_info->scheme_path = create_str(1024);
    system_info->sbcl_path = create_str(1024);
    system_info->nodejs_path = create_str(1024);
    system_info->java_xms = create_str(1024);
    system_info->java_xmx = create_str(1024);

    return system_info;
}

#endif //JUSTOJ_CORE_SYSTEM_INFO_H
