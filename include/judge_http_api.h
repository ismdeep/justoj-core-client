//
// Created by ismdeep on 2019/12/19.
//

#ifndef JUSTOJ_CORE_JUDGE_HTTP_API_H
#define JUSTOJ_CORE_JUDGE_HTTP_API_H

#include <ismdeep-c-utils/time.h>
#include <utils.h>
#include <oj_result_text.h>
#include <log.h>

static char lang_ext[20][8] = {"c", "cc", "pas", "java", "rb", "sh", "py",
                               "php", "pl", "cs", "m", "bas", "scm", "c", "cc", "lua", "js", "go", "py",
                               "cl"};

bool judge_http_api_check_secure_code(const char *http_base_url, const char *secure_code) {
    const char *cmd =
            "wget -t 3 --post-data=\"secure_code=%s\" -q -O - \"%s/api/judge_api/check_secure_code?_=%s\"";
    char* current_time_id_str = current_time_id();
    FILE *fjobs = read_cmd_output(cmd, secure_code, http_base_url, current_time_id_str);
    free(current_time_id_str);
    char ret[1024];
    fscanf(fjobs, "%s", ret);
    return (strcmp("1", ret) == 0);
}

int judge_http_api_get_jobs(const char *http_base_url, const char *secure_code, const char *oj_lang_set, int query_size,
                            int *jobs) {
    char buf[BUFFER_SIZE];
    const char *cmd = "wget --post-data=\"secure_code=%s&oj_lang_set=%s&query_size=%d\" -q -O - \"%s/api/judge_api/get_pending?_=%s\"";
    char* current_time_id_str = current_time_id();
    FILE *fjobs = read_cmd_output(cmd, secure_code, oj_lang_set, query_size, http_base_url, current_time_id_str);
    free(current_time_id_str);
    fscanf(fjobs, "%s", buf);
    if (strcmp(buf, "solution_ids") != 0) {
        pclose(fjobs);
        return 0;
    }
    int jobs_cnt = 0;
    while (fscanf(fjobs, "%s", buf) != EOF) {
        int sid = atoi(buf);
        if (sid > 1000)
            jobs[jobs_cnt++] = sid;
    }
    pclose(fjobs);
    return jobs_cnt;
}

bool judge_http_api_set_solution_result(const char *http_base_url, const char *secure_code, int solution_id, int result) {
    const char *cmd = "wget --post-data=\"secure_code=%s&sid=%d&result=%d\" -q -O - \"%s/api/judge_api/checkout?_=%s\"";
    char ret_str[1024] = "";
    while (1) {
        char* current_time_id_str = current_time_id();
        FILE *fjobs = read_cmd_output(cmd, secure_code, solution_id, result, http_base_url, current_time_id_str);
        free(current_time_id_str);
        fscanf(fjobs, "%s", ret_str);
        pclose(fjobs);
        if (strcmp(ret_str, "1") == 0) {
            break;
        }
        sleep(1);
    }

    return 1;
}

void judge_http_api_update_solution(const char *http_base_url, const char *secure_code, int solution_id, int result,
                                    int time, int memory) {
    char ret_str[1024] = "";
    const char *cmd =
            "wget --post-data=\"secure_code=%s&sid=%d&result=%d&time=%d&memory=%d\" -q -O - \"%s/api/judge_api/update_solution?_=%s\"";
    while (1) {
        char *time_id = current_time_id();
        FILE *fjobs = read_cmd_output(cmd, secure_code, solution_id, result, time, memory, http_base_url, time_id);
        fscanf(fjobs, "%s", ret_str);
        free(time_id);
        if (strcmp(ret_str, "1") == 0) {
            break;
        }
        pclose(fjobs);
        sleep(1);
    }
}


void judge_http_api_add_ce_info_http(const char *http_base_url, const char *secure_code, int solution_id) {
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
    fprintf(ce, "secure_code=%s&sid=%d&ceinfo=%s", secure_code, solution_id, ceinfo_encode);
    fclose(ce);
    free(ceinfo_encode);
    char ret_str[1024] = "";
    const char *cmd = "wget --post-file=\"ce.post\" -q -O - \"%s/api/judge_api/add_ce_info?_=%s\"";
    char* current_time_id_str = current_time_id();
    FILE *fjobs = read_cmd_output(cmd, http_base_url, current_time_id_str);
    free(current_time_id_str);
    fscanf(fjobs, "%s", ret_str);
    pclose(fjobs);
}


void judge_http_api_get_solution(const char *http_base_url, const char *secure_code, int solution_id, int lang) {
    char src_pth[BUFFER_SIZE];

    // create the src file
    sprintf(src_pth, "Main.%s", lang_ext[lang]);

    const char *cmd2 ="wget --post-data=\"secure_code=%s&sid=%d\" -q -O %s \"%s/api/judge_api/get_solution?_=%s\"";
    char *current_time_id_str = current_time_id();
    FILE *pout = read_cmd_output(cmd2, secure_code, solution_id, src_pth, http_base_url, current_time_id_str);
    free(current_time_id_str);
    pclose(pout);
}

void judge_http_api_get_solution_info(const char *http_base_url, const char *secure_code, int solution_id, int *p_id,
                                      char *user_id, int *lang) {
    const char *cmd = "wget --post-data=\"secure_code=%s&sid=%d\" -q -O - \"%s/api/judge_api/get_solution_info?_=%s\"";
    char *current_time_id_str = current_time_id();
    FILE *pout = read_cmd_output(cmd, secure_code, solution_id, http_base_url, current_time_id_str);
    fscanf(pout, "%d", p_id);
    fscanf(pout, "%s", user_id);
    fscanf(pout, "%d", lang);
    pclose(pout);
    free(current_time_id_str);
}

void _get_problem_info_http(const char *http_base_url, const char *secure_code, int p_id, int *time_lmt, int *mem_lmt,
                            int *isspj) {
    const char *cmd =
            "wget --post-data=\"secure_code=%s&pid=%d\" -q -O - \"%s/api/judge_api/get_problem_info\"";
    FILE *pout = read_cmd_output(cmd, secure_code, p_id, http_base_url);
    fscanf(pout, "%d", time_lmt);
    fscanf(pout, "%d", mem_lmt);
    fscanf(pout, "%d", isspj);
    pclose(pout);
}

/************************************************************************************/


void _update_user_http(const char *http_base_url, const char *secure_code, char *user_id) {
    const char *cmd =
            " wget --post-data=\"secure_code=%s&updateuser=1&user_id=%s\" -q -O - \"%s/admin/problem_judge.php\"";
    FILE *fjobs = read_cmd_output(cmd, secure_code, user_id, http_base_url);
    pclose(fjobs);
}

void _update_problem_http(const char *http_base_url, const char *secure_code, int problem_id) {
    const char *cmd =
            " wget --post-data=\"secure_code=%s&updateproblem=1&pid=%d\" -q -O - \"%s/admin/problem_judge.php\"";
    FILE *fjobs = read_cmd_output(cmd, secure_code, problem_id, http_base_url);
    pclose(fjobs);
}

#endif //JUSTOJ_CORE_JUDGE_HTTP_API_H
