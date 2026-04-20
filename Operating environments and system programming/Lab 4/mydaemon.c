#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_TRACKED 64
#define DEFAULT_CONFIG "daemon.conf"
#define DEFAULT_LOG    "daemon.log"
#define DEFAULT_PID    "mydaemon.pid"

typedef struct {
    const char *name;
    int signo;
} SignalMap;

static const SignalMap SIGNALS[] = {
    {"SIGHUP",  SIGHUP},
    {"SIGINT",  SIGINT},
    {"SIGQUIT", SIGQUIT},
    {"SIGUSR1", SIGUSR1},
    {"SIGUSR2", SIGUSR2},
    {"SIGTERM", SIGTERM},
    {"SIGCHLD", SIGCHLD},
#ifdef SIGTSTP
    {"SIGTSTP", SIGTSTP},
#endif
#ifdef SIGCONT
    {"SIGCONT", SIGCONT},
#endif
#ifdef SIGPIPE
    {"SIGPIPE", SIGPIPE},
#endif
#ifdef SIGALRM
    {"SIGALRM", SIGALRM},
#endif
    {NULL, 0}
};

#define MAX_SIGNAL_NUM 128
static bool tracked[MAX_SIGNAL_NUM];
static char config_path[512] = DEFAULT_CONFIG;
static char log_path[512] = DEFAULT_LOG;
static char pid_path[512] = DEFAULT_PID;
static int pid_fd = -1;

static const char *signal_to_name(int signo) {
    for (size_t i = 0; SIGNALS[i].name != NULL; ++i) {
        if (SIGNALS[i].signo == signo) return SIGNALS[i].name;
    }
    return "UNKNOWN";
}

static int name_to_signal(const char *name) {
    for (size_t i = 0; SIGNALS[i].name != NULL; ++i) {
        if (strcmp(SIGNALS[i].name, name) == 0) return SIGNALS[i].signo;
    }
    return -1;
}

static void trim(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' || s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[--len] = '\0';
    }
    char *start = s;
    while (*start == ' ' || *start == '\t') start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
}

static void log_message(const char *fmt, ...) {
    FILE *f = fopen(log_path, "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);

    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm_now);
    fprintf(f, "[%s] ", timebuf);

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);

    fputc('\n', f);
    fclose(f);
}

static int load_config(void) {
    FILE *f = fopen(config_path, "r");
    if (!f) {
        log_message("ERROR: cannot open config '%s': %s", config_path, strerror(errno));
        return -1;
    }

    for (int i = 0; i < MAX_SIGNAL_NUM; ++i) tracked[i] = false;

    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        int signo = name_to_signal(line);
        if (signo == -1) {
            log_message("WARNING: unknown signal in config: %s", line);
            continue;
        }
        if (signo >= 0 && signo < MAX_SIGNAL_NUM) tracked[signo] = true;
        count++;
    }

    fclose(f);
    log_message("Config reloaded from '%s'. Active signals count: %d", config_path, count);
    return 0;
}

static int write_pid_file(void) {
    pid_fd = open(pid_path, O_RDWR | O_CREAT, 0644);
    if (pid_fd == -1) {
        return -1;
    }

    if (flock(pid_fd, LOCK_EX | LOCK_NB) == -1) {
        close(pid_fd);
        pid_fd = -1;
        return -1;
    }

    if (ftruncate(pid_fd, 0) == -1) return -1;

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%ld\n", (long)getpid());
    if (write(pid_fd, buf, (size_t)len) != len) return -1;

    return 0;
}

static void remove_pid_file(void) {
    if (pid_fd != -1) {
        close(pid_fd);
        pid_fd = -1;
    }
    unlink(pid_path);
}

static int read_pid_from_file(pid_t *pid) {
    FILE *f = fopen(pid_path, "r");
    if (!f) return -1;
    long value = -1;
    int rc = fscanf(f, "%ld", &value);
    fclose(f);
    if (rc != 1 || value <= 0) return -1;
    *pid = (pid_t)value;
    return 0;
}


static void make_absolute_path(char *path, size_t size) {
    if (path[0] == '/') return;

    char cwd[1024];
    if (!getcwd(cwd, sizeof(cwd))) return;

    char temp[1024];
    snprintf(temp, sizeof(temp), "%s/%s", cwd, path);
    strncpy(path, temp, size - 1);
    path[size - 1] = '\0';
}

static int daemonize_process(void) {
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() == -1) return -1;

    pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    if (chdir("/") == -1) return -1;

    int fd = open("/dev/null", O_RDWR);
    if (fd == -1) return -1;

    if (dup2(fd, STDIN_FILENO) == -1) return -1;
    if (dup2(fd, STDOUT_FILENO) == -1) return -1;
    if (dup2(fd, STDERR_FILENO) == -1) return -1;
    if (fd > STDERR_FILENO) close(fd);

    return 0;
}

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage:\n"
            "  %s [-c config] [-l log] [-p pid]\n"
            "  %s -q [-p pid]\n\n"
            "Options:\n"
            "  -c FILE   config path\n"
            "  -l FILE   log path\n"
            "  -p FILE   pid file path\n"
            "  -q        stop running daemon\n",
            prog, prog);
}

int main(int argc, char *argv[]) {
    bool quit_mode = false;

    int opt;
    while ((opt = getopt(argc, argv, "c:l:p:q")) != -1) {
        switch (opt) {
            case 'c':
                strncpy(config_path, optarg, sizeof(config_path) - 1);
                config_path[sizeof(config_path) - 1] = '\0';
                break;
            case 'l':
                strncpy(log_path, optarg, sizeof(log_path) - 1);
                log_path[sizeof(log_path) - 1] = '\0';
                break;
            case 'p':
                strncpy(pid_path, optarg, sizeof(pid_path) - 1);
                pid_path[sizeof(pid_path) - 1] = '\0';
                break;
            case 'q':
                quit_mode = true;
                break;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    make_absolute_path(config_path, sizeof(config_path));
    make_absolute_path(log_path, sizeof(log_path));
    make_absolute_path(pid_path, sizeof(pid_path));

    if (quit_mode) {
        pid_t pid;
        if (read_pid_from_file(&pid) == -1) {
            fprintf(stderr, "Cannot read pid file: %s\n", pid_path);
            return EXIT_FAILURE;
        }
        if (kill(pid, SIGTERM) == -1) {
            perror("kill");
            return EXIT_FAILURE;
        }
        printf("SIGTERM sent to daemon pid %ld\n", (long)pid);
        return EXIT_SUCCESS;
    }

    sigset_t mask;
    sigemptyset(&mask);

    for (size_t i = 0; SIGNALS[i].name != NULL; ++i) {
        sigaddset(&mask, SIGNALS[i].signo);
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("sigprocmask");
        return EXIT_FAILURE;
    }

    if (daemonize_process() == -1) {
        return EXIT_FAILURE;
    }

    if (write_pid_file() == -1) {
        log_message("ERROR: cannot create or lock pid file '%s'", pid_path);
        return EXIT_FAILURE;
    }

    if (load_config() == -1) {
        remove_pid_file();
        return EXIT_FAILURE;
    }

    log_message("Daemon started. PID=%ld", (long)getpid());

    for (;;) {
        int signo = 0;
        int rc = sigwait(&mask, &signo);
        if (rc != 0) {
            log_message("ERROR: sigwait failed: %s", strerror(rc));
            continue;
        }

        if (signo >= 0 && signo < MAX_SIGNAL_NUM && tracked[signo]) {
            log_message("Received signal %s (%d)", signal_to_name(signo), signo);
        }

        if (signo == SIGHUP) {
            load_config();
        }

        if (signo == SIGTERM) {
            log_message("Daemon shutting down by SIGTERM");
            break;
        }
    }

    remove_pid_file();
    return EXIT_SUCCESS;
}
