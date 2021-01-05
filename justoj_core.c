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
#include <ismdeep-c-utils/argv.h>

#include <log.h>

#include <version.h>

#include <solution_queue.h>

#include <system_info.h>

#include <data_monitor.h>
#include <heartbeat.h>

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)


static char lock_file[BUFFER_SIZE];

static bool STOP = false;

struct SolutionQueue *queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

struct SystemInfo *system_info;

void call_for_exit() {
    STOP = true;
    printf("Signal: STOPPING.\n");
    printf("Signal: STOPPING.\n");
    printf("Signal: STOPPING.\n");
    log_info("Stopping judged...");
}

/* Init variables from config file */
void init_judge_conf() {
    FILE *fp = NULL;
    char buf[BUFFER_SIZE];
    char config_file_path[BUFFER_SIZE];
    sprintf(config_file_path, "%s/etc/judge.conf", system_info->oj_home);
    fp = fopen(config_file_path, "r");

    if (fp == NULL) {
        log_info("Config File not found!! [%s]", config_file_path);
        exit(-1);
    }

    while (fgets(buf, BUFFER_SIZE - 1, fp)) {
        read_int(buf, "OJ_RUNNING", &system_info->max_running);
        read_int(buf, "OJ_QUERY_SIZE", &system_info->query_size);
        read_int(buf, "OJ_SLEEP_TIME", &system_info->sleep_time);
        read_buf(buf, "OJ_CLIENT_NAME", system_info->client_name);
        read_buf(buf, "OJ_SECURE_CODE", system_info->secure_code);
        read_buf(buf, "OJ_HTTP_BASE_URL", system_info->http_base_url);
        read_buf(buf, "OJ_LANG_SET", system_info->oj_lang_set);
        read_buf(buf, "CLIENT", system_info->client_path);
        read_buf(buf, "DATA", system_info->data_path);
    }
}

void run_client(int solution_id) {
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
    execute_cmd("%s %s %d", system_info->client_path, system_info->oj_home, solution_id);
    log_info("%d DONE", solution_id);
}


void fetch_solution_ids() {
    int *solution_ids = (int *) malloc(sizeof(int) * system_info->query_size);

    while (true) {
        if (queue->size >= system_info->query_size) {
            if (STOP) {
                break;
            }

            SLEEP_MS(100);
            continue;
        }

        /* get the database info */
        int ret = get_pending_solutions(system_info, solution_ids);
        if (0 == ret) {
            if (STOP) {
                break;
            }
            continue;
        }

        for (int i = 0; i < system_info->query_size; i++) {
            if (solution_ids[i] >= 1000) {
                while (!solution_queue_push(queue, solution_ids[i])) {
                    SLEEP_MS(100);
                }
                log_info("%d ADDED", solution_ids[i]);
            }
        }
    }

    log_info("%p => fetch_solution_ids() STOPPED.", pthread_self());
}


void working_solution() {
    int solution_id;
    while (true) {
        solution_id = solution_queue_pop(queue);
        if (solution_id < 1000) {
            if (STOP) {
                break;
            }
            SLEEP_MS(100);
            continue;
        }

        run_client(solution_id);
    }

    log_info("%p => working_solution() STOPPED.", pthread_self());
}


void work() {
    log_info("query_size : %d", system_info->query_size);
    log_info("max_running: %d", system_info->max_running);

    /* 初始化队列 */
    queue = solution_queue_create(system_info->query_size * 4);
    queue->mutex = &queue_mutex;

    /* 启动生产者线程 */
    pthread_t *fetch_thread = (pthread_t *) malloc(sizeof(pthread_t) * 1);
    pthread_create(fetch_thread, NULL, (void *(*)(void *)) fetch_solution_ids, NULL);

    /* 启动消费者线程们 */
    pthread_t *working_threads = (pthread_t *) malloc(sizeof(pthread_t) * system_info->max_running);
    for (int i = 0; i < system_info->max_running; i++) {
        pthread_create(&working_threads[i], NULL, (void *(*)(void *)) working_solution, NULL);
    }

    /* 阻塞进程 */
    pthread_join(*fetch_thread, NULL);
    for (int i = 0; i < system_info->max_running; i++) {
        pthread_join(working_threads[i], NULL);
    }

    /* 清理 */
    free(fetch_thread);
    free(working_threads);
    solution_queue_destroy(queue);
    queue->mutex = NULL;
    queue = NULL;
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
    chdir(system_info->oj_home); /* change working directory */
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

void data_monitor_worker() {
    while (!STOP) {
        data_monitor(system_info);
        SLEEP_S(60);
    }
}

void heartbeat_worker() {
    while (!STOP) {
        heartbeat(system_info);
        SLEEP_S(60);
    }
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
        printf("VERSION: %s\n", get_version());
        printf("Usage: %s ${base_path}\n", argv[0]);
        return -1;
    }

    if (argv_exist_switch(argc, argv, "--version")) {
        printf("justoj-core        version: %s\n", get_version());
        return EXIT_SUCCESS;
    }

    system_info = system_info_create();

    /* Set base path (oj_home) */
    strcpy(system_info->oj_home, argv[1]);
    chdir(system_info->oj_home);
    sprintf(lock_file, "%s/lock.pid", system_info->oj_home);

    char log_file_path[1024];
    sprintf(log_file_path, "%s/product.log", system_info->oj_home);

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


    /* Read judge.conf */
    init_judge_conf();

    if (strcmp(system_info->client_path, "") == 0) {
        log_error("CLIENT is EMPTY. [%s]", system_info->client_path);
        return EXIT_FAILURE;
    }

    log_info("VERSION      : %s", get_version());
    log_info("URL_BASE     : %s", system_info->http_base_url);
    log_info("OJ_HOME      : %s", system_info->oj_home);
    log_info("CLIENT NAME  : %s", system_info->client_name);
    log_info("CLIENT PATH  : %s", system_info->client_path);

    if (!check_secure_code(system_info)) {
        log_info("ERROR: Secure code is invalid.");
        log_info("JustOJ Core STOPPED");
        return EXIT_FAILURE;
    }
    log_info("Check secure code successfully.");

    system("/sbin/iptables -A OUTPUT -m owner --uid-owner judge -j DROP");

    signal(SIGINT, call_for_exit);
    signal(SIGUSR1, call_for_exit);

    // Run send justoj-data git bash info thread
    pthread_t data_monitor_thread;
    pthread_create(&data_monitor_thread, NULL, (void *(*)(void *)) data_monitor_worker, NULL);

    log_info("Start to working... ");
    work();
    log_info("JustOJ Core STOPPED");


    return EXIT_SUCCESS;
}
