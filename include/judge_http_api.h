//
// Created by ismdeep on 2019/12/19.
//

#ifndef JUSTOJ_CORE_JUDGE_HTTP_API_H
#define JUSTOJ_CORE_JUDGE_HTTP_API_H

#include <ismdeep-c-utils/time.h>
#include <utils.h>
#include <oj_result_text.h>
#include <log.h>

#include <wget_utils.h>

#include <system_info.h>
#include <solution_info.h>

static char lang_ext[20][8] = {"c", "cc", "pas", "java", "rb", "sh", "py",
                               "php", "pl", "cs", "m", "bas", "scm", "c", "cc", "lua", "js", "go", "py",
                               "cl"};

bool judge_http_api_check_secure_code(const char *http_base_url, const char *secure_code) {
    char *time_id = current_time_id();
    char *url = (char *) malloc(sizeof(char) * 1024);
    char *post_data = (char *) malloc(sizeof(char) * 4096);
    memset(url, 0, sizeof(char) * 1024);
    memset(post_data, 0, sizeof(char) * 4096);
    sprintf(url, "%s/api/judge_api/check_secure_code?_=%s", http_base_url, time_id);
    sprintf(post_data, "secure_code=%s", secure_code);
    free(time_id);

    FILE *fp = wget_post_fp(url, post_data);
    char *ret = (char *) malloc(sizeof(char) * 1024);
    memset(ret, 0, sizeof(char) * 1024);
    fscanf(fp, "%s", ret);
    return strcmp(ret, "1") == 0 ? true : false;
}

int judge_http_api_get_solutions(
        const char *http_base_url,
        const char *secure_code,
        const char *oj_lang_set,
        int query_size,
        int *solution_ids) {
    char *url = (char *) malloc(sizeof(char) * 1024);
    char *post_data = (char *) malloc(sizeof(char) * 4096);
    memset(url, 0, sizeof(char) * 1024);
    memset(post_data, 0, sizeof(char) * 4096);

    char *time_id = current_time_id();
    sprintf(url, "%s/api/judge_api/get_pending?_=%s", http_base_url, time_id);
    sprintf(post_data, "secure_code=%s&oj_lang_set=%s&query_size=%d", secure_code, oj_lang_set, query_size);
    free(time_id);

    FILE *fp = wget_post_fp(url, post_data);
    free(url);
    free(post_data);

    char *buf = (char *) malloc(sizeof(char) * 1024);

    fscanf(fp, "%s", buf);
    if (strcmp(buf, "solution_ids") != 0) {
        pclose(fp);
        free(buf);
        return 0;
    }

    int jobs_cnt = 0;
    for (int i = 0; i < query_size; i++) {
        solution_ids[i] = 0;
    }
    while (fscanf(fp, "%s", buf) != EOF) {
        int solution_id = atoi(buf);
        if (solution_id > 1000)
            solution_ids[jobs_cnt++] = solution_id;
    }
    free(buf);
    pclose(fp);
    return jobs_cnt;
}

void judge_http_api_update_solution(
        const struct SystemInfo *system_info,
        const struct SolutionInfo *solution_info
//        const char *http_base_url, const char *secure_code, int solution_id, int result,
//                                    int time, int memory
                                    ) {
    char ret_str[1024] = "";
    const char *cmd =
            "wget --post-data=\"secure_code=%s&sid=%d&result=%d&time=%d&memory=%d\" -q -O - \"%s/api/judge_api/update_solution?_=%s\"";
    while (1) {
        char *time_id = current_time_id();
        FILE *fjobs = read_cmd_output(cmd,
                                      system_info->secure_code,
                                      solution_info->solution_id,
                                      solution_info->result,
                                      solution_info->result_time,
                                      solution_info->result_memory,
                                      system_info->http_base_url,
                                      time_id);
        fscanf(fjobs, "%s", ret_str);
        free(time_id);
        if (strcmp(ret_str, "1") == 0) {
            break;
        }
        pclose(fjobs);
        sleep(1);
    }
}


void judge_http_api_add_ce_info_http(const struct SystemInfo *system_info, struct SolutionInfo *solution_info
//        const char *http_base_url, const char *secure_code, int solution_id
        ) {
    char ceinfo[(1 << 16)], *cend;
    char *ceinfo_encode;
    FILE *fp = fopen("ce.txt", "re");

    cend = ceinfo;
    while (fgets(cend, 1024, fp)) {
        cend += strlen(cend);
        if (cend - ceinfo > 40000)
            break;
    }
    fclose(fp);
    ceinfo_encode = url_encode(ceinfo);
    FILE *ce = fopen("ce.post", "we");
    fprintf(ce, "secure_code=%s&sid=%d&ceinfo=%s", system_info->secure_code, solution_info->solution_id, ceinfo_encode);
    fclose(ce);
    free(ceinfo_encode);
    char ret_str[1024] = "";
    const char *cmd = "wget --post-file=\"ce.post\" -q -O - \"%s/api/judge_api/add_ce_info?_=%s\"";
    char *current_time_id_str = current_time_id();
    FILE *fjobs = read_cmd_output(cmd, system_info->http_base_url, current_time_id_str);
    free(current_time_id_str);
    fscanf(fjobs, "%s", ret_str);
    pclose(fjobs);
}


void judge_http_api_download_source_code(const struct SystemInfo *_system_info_, struct SolutionInfo *_solution_info_) {
    // create the src file
    sprintf(_solution_info_->src_path, "Main.%s", lang_ext[_solution_info_->lang_id]);

    const char *cmd2 = "wget --post-data=\"secure_code=%s&sid=%d\" -q -O %s \"%s/api/judge_api/get_solution?_=%s\"";
    char *current_time_id_str = current_time_id();
    FILE *pout = read_cmd_output(cmd2,
                                 _system_info_->secure_code,
                                 _solution_info_->solution_id,
                                 _solution_info_->src_path,
                                 _system_info_->http_base_url,
                                 current_time_id_str);
    free(current_time_id_str);
    pclose(pout);
}

void judge_http_api_get_solution_info(
        const struct SystemInfo *system_info,
        struct SolutionInfo *solution_info
) {
    const char *cmd = "wget --post-data=\"secure_code=%s&sid=%d\" -q -O - \"%s/api/judge_api/get_solution_info?_=%s\"";
    char *current_time_id_str = current_time_id();
    FILE *pout = read_cmd_output(cmd, system_info->secure_code, solution_info->solution_id, system_info->http_base_url,
                                 current_time_id_str);
    fscanf(pout, "%d", &solution_info->problem_id);
    fscanf(pout, "%s", solution_info->user_id);
    fscanf(pout, "%d", &solution_info->lang_id);
    pclose(pout);
    free(current_time_id_str);
}

void judge_http_api_get_problem_info(const struct SystemInfo *_system_info_, struct SolutionInfo *_solution_info_) {
    const char *cmd = "wget --post-data=\"secure_code=%s&pid=%d\" -q -O - \"%s/api/judge_api/get_problem_info\"";
    FILE *pout = read_cmd_output(cmd,
                                 _system_info_->secure_code,
                                 _solution_info_->problem_id,
                                 _system_info_->http_base_url);
    fscanf(pout, "%d", &_solution_info_->time_lmt);
    fscanf(pout, "%d", &_solution_info_->mem_lmt);
    int tmp;
    fscanf(pout, "%d", &tmp);
    _solution_info_->is_special_judge = tmp ? true : false;
    pclose(pout);

    if (_solution_info_->time_lmt <= 0) {
        _solution_info_->time_lmt = 1;
    }

}

#endif //JUSTOJ_CORE_JUDGE_HTTP_API_H
