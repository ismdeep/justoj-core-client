//
// Created by ismdeep on 2019/12/19.
//

#ifndef JUSTOJ_CORE_JUDGE_HTTP_API_H
#define JUSTOJ_CORE_JUDGE_HTTP_API_H

#include <inttypes.h>

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

bool check_secure_code(const struct SystemInfo *system_info) {
    char *url = (char *) malloc(sizeof(char) * 1024);
    char *post_data = (char *) malloc(sizeof(char) * 4096);
    memset(url, 0, sizeof(char) * 1024);
    memset(post_data, 0, sizeof(char) * 4096);
    sprintf(url, "%s/api/judge_api/check_secure_code?client_name=%s&_=%"PRIu64,
            system_info->http_base_url,
            system_info->client_name,
            time_us());
    sprintf(post_data, "secure_code=%s", system_info->secure_code);

    FILE *fp = wget_post_fp(url, post_data);
    char *ret = (char *) malloc(sizeof(char) * 1024);
    memset(ret, 0, sizeof(char) * 1024);
    fscanf(fp, "%s", ret);
    return strcmp(ret, "1") == 0 ? true : false;
}

int get_pending_solutions(const struct SystemInfo *system_info, int *solution_ids) {
    char *url = (char *) malloc(sizeof(char) * 1024);
    char *post_data = (char *) malloc(sizeof(char) * 4096);
    memset(url, 0, sizeof(char) * 1024);
    memset(post_data, 0, sizeof(char) * 4096);

    sprintf(url, "%s/api/judge_api/get_pending?client_name=%s&_=%"PRIu64,
            system_info->http_base_url,
            system_info->client_name,
            time_us());
    sprintf(post_data, "secure_code=%s&oj_lang_set=%s&query_size=%d",
            system_info->secure_code,
            system_info->oj_lang_set,
            system_info->query_size);

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
    for (int i = 0; i < system_info->query_size; i++) {
        solution_ids[i] = 0;
    }
    while (fscanf(fp, "%s", buf) != EOF) {
        int solution_id = strtol(buf, NULL, 10);
        if (solution_id > 1000)
            solution_ids[jobs_cnt++] = solution_id;
    }
    free(buf);
    pclose(fp);
    return jobs_cnt;
}

void update_solution(const struct SystemInfo *system_info, const struct SolutionInfo *solution_info) {
    char ret_str[1024] = "";
    const char *cmd = "wget "
                      "--post-data=\"secure_code=%s&sid=%d&result=%d&time=%d&memory=%d\" "
                      "-q -O - "
                      "\"%s/api/judge_api/update_solution?client_name=%s&_=%"PRIu64"\"";
    while (1) {
        FILE *fjobs = read_cmd_output(cmd,
                                      system_info->secure_code,
                                      solution_info->solution_id,
                                      solution_info->result,
                                      solution_info->result_time,
                                      solution_info->result_memory,
                                      system_info->http_base_url,
                                      system_info->client_name,
                                      time_us());
        fscanf(fjobs, "%s", ret_str);
        if (strcmp(ret_str, "1") == 0) {
            break;
        }
        pclose(fjobs);
        sleep(1);
    }
}


void upload_ce_info(const struct SystemInfo *system_info, struct SolutionInfo *solution_info) {
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
    const char *cmd = "wget --post-file=\"ce.post\" "
                      "-q -O - "
                      "\"%s/api/judge_api/add_ce_info?client_name=%s&_=%"PRIu64"\"";
    FILE *fjobs = read_cmd_output(cmd,
                                  system_info->http_base_url,
                                  system_info->client_name,
                                  time_us());
    fscanf(fjobs, "%s", ret_str);
    pclose(fjobs);
}


void download_source_code(const struct SystemInfo *system_info, struct SolutionInfo *solution_info) {
    // create the src file
    sprintf(solution_info->src_path, "Main.%s", lang_ext[solution_info->lang_id]);

    const char *cmd2 = "wget --post-data=\"secure_code=%s&sid=%d\" "
                       "-q -O %s "
                       "\"%s/api/judge_api/get_solution?client_name=%s&_=%"PRIu64"\"";
    FILE *pout = read_cmd_output(cmd2,
                                 system_info->secure_code,
                                 solution_info->solution_id,
                                 solution_info->src_path,
                                 system_info->http_base_url,
                                 system_info->client_name,
                                 time_us());
    pclose(pout);
}

void get_solution_info(const struct SystemInfo *system_info, struct SolutionInfo *solution_info) {
    const char *cmd = "wget --post-data=\"secure_code=%s&sid=%d\" "
                      "-q -O - "
                      "\"%s/api/judge_api/get_solution_info?client_name=%s&_=%"PRIu64"\"";
    FILE *pout = read_cmd_output(cmd,
                                 system_info->secure_code,
                                 solution_info->solution_id,
                                 system_info->http_base_url,
                                 system_info->client_name,
                                 time_us());
    fscanf(pout, "%d", &solution_info->problem_id);
    fscanf(pout, "%s", solution_info->user_id);
    fscanf(pout, "%d", &solution_info->lang_id);
    pclose(pout);
}

void get_problem_info(const struct SystemInfo *system_info, struct SolutionInfo *solution_info) {
    const char *cmd = "wget --post-data=\"secure_code=%s&pid=%d\" "
                      "-q -O - "
                      "\"%s/api/judge_api/get_problem_info?client_name=%s&_=%"PRIu64"\"";
    FILE *pout = read_cmd_output(cmd,
                                 system_info->secure_code,
                                 solution_info->problem_id,
                                 system_info->http_base_url,
                                 system_info->client_name,
                                 time_us());
    fscanf(pout, "%d", &solution_info->time_lmt);
    fscanf(pout, "%d", &solution_info->mem_lmt);
    int tmp;
    fscanf(pout, "%d", &tmp);
    solution_info->is_special_judge = tmp ? true : false;
    pclose(pout);

    if (solution_info->time_lmt <= 0) {
        solution_info->time_lmt = 1;
    }

}

#endif //JUSTOJ_CORE_JUDGE_HTTP_API_H
