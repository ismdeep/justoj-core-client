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
    char *oj_home;
    char *client_name;
    char *http_base_url;
    char *secure_code;

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
    system_info->http_base_url = create_str(1024);
    system_info->secure_code = create_str(1024);
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
