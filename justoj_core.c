//
// Created by L. Jiang <l.jiang.1024@gmail.com> on 2019/12/18.
//

#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/resource.h>

#include <judge_http_api.h>

#include <ismdeep-c-utils/threadpool.h>
#include <ismdeep-c-utils/string.h>
#include <ismdeep-c-utils/argv.h>

#include <log.h>

#include <version.h>

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define STD_MB 1048576

#define TIMES(id, size) for(int id = 0; id < (size); ++id)

static char lock_file[BUFFER_SIZE];
static char secure_code[BUFFER_SIZE];
static char base_path[BUFFER_SIZE];
static char client_name[BUFFER_SIZE];
static char oj_lang_set[BUFFER_SIZE];
static int max_running;
static int query_size;
static int sleep_time;
static char http_base_url[BUFFER_SIZE];

static bool STOP = false;

void call_for_exit() {
    STOP = true;
    log_info("Stopping judged...");
}

void call_for_status() {
    log_info("Running");
}

/* Init variables from config file */
void init_judge_conf() {
    FILE *fp = NULL;
    char buf[BUFFER_SIZE];
    max_running = 4;
    sleep_time = 1;
    strcpy(oj_lang_set, "0,1,2,3,4,5,6,7,8,9,10");

    char config_file_path[BUFFER_SIZE];
    sprintf(config_file_path, "%s/etc/judge.conf", base_path);

    fp = fopen(config_file_path, "r");
    if (fp != NULL) {
        while (fgets(buf, BUFFER_SIZE - 1, fp)) {
            read_int(buf, "OJ_RUNNING", &max_running);
            read_int(buf, "OJ_QUERY_SIZE", &query_size);
            read_int(buf, "OJ_SLEEP_TIME", &sleep_time);
            read_buf(buf, "OJ_CLIENT_NAME", client_name);
            read_buf(buf, "OJ_SECURE_CODE", secure_code);
            read_buf(buf, "OJ_HTTP_BASE_URL", http_base_url);
            read_buf(buf, "OJ_LANG_SET", oj_lang_set);
        }
    } else {
        log_info("Config File not found!! [%s]", config_file_path);
    }
}

void run_client(char *solution_id_str) {
    struct rlimit LIM;
    LIM.rlim_max = 800;
    LIM.rlim_cur = 800;
    setrlimit(RLIMIT_CPU, &LIM);

    LIM.rlim_max = 80 * STD_MB;
    LIM.rlim_cur = 80 * STD_MB;
    setrlimit(RLIMIT_FSIZE, &LIM);

    LIM.rlim_max = STD_MB << 11;
    LIM.rlim_cur = STD_MB << 11;
    setrlimit(RLIMIT_AS, &LIM);

    LIM.rlim_cur = LIM.rlim_max = 200;
    setrlimit(RLIMIT_NPROC, &LIM);
    execute_cmd("justoj-core-client %s %s", base_path, solution_id_str);
    free(solution_id_str);
}

void *run_client_thread_func(void *arg) {
    char *solution_id_str = (char *) arg;
    run_client(solution_id_str);
    return NULL;
}

void work() {
    int solution_ids[query_size];
    TIMES(i, query_size) {
        solution_ids[i] = 0;
    }
    /* get the database info */
    if (0 == judge_http_api_get_jobs(http_base_url, secure_code, oj_lang_set, query_size, solution_ids)) {
        return;
    }

    struct ThreadPool *pool = threadpool_init(max_running, query_size);

    TIMES(i, query_size) {
        if (solution_ids[i] >= 1000) {
            threadpool_add_job(pool, run_client_thread_func, int_to_string(solution_ids[i]));
        }
    }

    threadpool_destroy(pool);
}

int lockfile(int fd) {
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return (fcntl(fd, F_SETLK, &fl));
}

int already_running() {
    int fd;
    char buf[16];
    fd = open(lock_file, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) {
        log_info("can't open %s: %s", lock_file, strerror(errno));
        exit(1);
    }
    if (lockfile(fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return 1;
        }
        log_info("can't lock %s: %s", lock_file, strerror(errno));
        exit(1);
    }
    ftruncate(fd, 0);
    sprintf(buf, "%d", getpid());
    write(fd, buf, strlen(buf) + 1);
    return 0;
}

int daemon_init(void) {
    pid_t pid;

    if ((pid = fork()) < 0)
        return (-1);

    else if (pid != 0)
        exit(0); /* parent exit */

    /* child continues */
    setsid(); /* become session leader */
    chdir(base_path); /* change working directory */
    umask(0); /* clear file mode creation mask */
    close(0); /* close stdin */
    close(1); /* close stdout */
    close(2); /* close stderr */
    int fd = open("/dev/null", O_RDWR);
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    if (fd > 2) {
        close(fd);
    }
    return (0);
}

/**
 * main() Function
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s ${base_path}", argv[0]);
        return -1;
    }

    /* Set base path (oj_home) */
    strcpy(base_path, argv[1]);
    chdir(base_path);
    sprintf(lock_file, "%s/lock.pid", base_path);

    char log_file_path[1024];
    sprintf(log_file_path, "%s/product.log", base_path);

    /* Determine if it is running under daemon mode. */
    bool daemon_flag = false;
    if (argv_exist_switch(argc, argv, "-d")) {
        daemon_flag = true;
    }
    if (daemon_flag) {
        daemon_init();
    }

    /* Judge if it is already running */
    if (already_running()) {
        log_info("VERSION: %s", get_version());
        log_warn("This daemon program [justoj-core] is already running!");
        return 1;
    }

    FILE *log_file = fopen(log_file_path, "ab");
    log_add_fp(log_file, LOG_INFO);
    log_set_quiet(true);

    log_info("VERSION: %s", get_version());
    log_info("JustOJ Core Started");
    log_info("OJ_HOME: [%s]", base_path);
    log_info("Lock PID File: [%s]", lock_file);

    /* Read judge.conf */
    init_judge_conf();

    log_info("Check secure code");
    if (!judge_http_api_check_secure_code(http_base_url, secure_code)) {
        log_info("Secure code is invalid.");
        log_info("JustOJ Core STOPPED");
        return 1;
    }
    log_info("Check secure code successfully.");

    system("/sbin/iptables -A OUTPUT -m owner --uid-owner judge -j DROP");

    signal(SIGINT, call_for_exit);
    signal(SIGUSR1, call_for_exit);
    signal(SIGUSR2, call_for_status);

    log_info("Start to working... ");

    while (!STOP) {
        work();
        sleep(sleep_time);
    }

    log_info("JustOJ Core STOPPED");
    return 0;
}
