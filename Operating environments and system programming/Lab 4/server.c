#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdarg.h>

#define SERVER_FIFO "/tmp/server_fifo"
#define LOG_FILE    "server.log"
#define MAX_BUFFER  1024

void log_message(pid_t client_pid, const char *fmt, ...) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log, "%s [%s] ", time_buf, (client_pid == 0 ? "SERVER" : "CLIENT"));

    va_list args;
    va_start(args, fmt);
    vfprintf(log, fmt, args);
    va_end(args);

    fprintf(log, "\n");
    fflush(log);
    fclose(log);
}

void sigint_handler(int sig) {
    log_message(0, "Server shutting down, removing FIFO...");
    unlink(SERVER_FIFO);
    exit(0);
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void process_request(pid_t client_pid, const char *command, const char *payload,
                     char *response, size_t resp_size) {
    if (strcmp(command, "ECHO") == 0) {
        snprintf(response, resp_size, "ECHO: %s", payload);
    } else if (strcmp(command, "UPPER") == 0) {
        char *upper = strdup(payload);
        for (int i = 0; upper[i]; i++) upper[i] = toupper(upper[i]);
        snprintf(response, resp_size, "UPPER: %s", upper);
        free(upper);
    } else if (strcmp(command, "LOWER") == 0) {
        char *lower = strdup(payload);
        for (int i = 0; lower[i]; i++) lower[i] = tolower(lower[i]);
        snprintf(response, resp_size, "LOWER: %s", lower);
        free(lower);
    } else if (strcmp(command, "TIME") == 0) {
        time_t now = time(NULL);
        snprintf(response, resp_size, "TIME: %s", ctime(&now));
        response[strlen(response)-1] = '\0';
    } else if (strcmp(command, "CALC") == 0) {
        int a, b;
        char op;
        if (sscanf(payload, "%d %c %d", &a, &op, &b) == 3) {
            int result = 0;
            switch (op) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': result = b != 0 ? a / b : 0; break;
                default:
                    snprintf(response, resp_size, "CALC: Unsupported operator");
                    return;
            }
            snprintf(response, resp_size, "CALC: %d %c %d = %d", a, op, b, result);
        } else {
            snprintf(response, resp_size, "CALC: Invalid format. Use: number operator number");
        }
    } else {
        snprintf(response, resp_size, "ERROR: Unknown command '%s'", command);
    }
}

int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    if (mkfifo(SERVER_FIFO, 0666) < 0 && errno != EEXIST) {
        perror("mkfifo");
        exit(1);
    }

    int server_fd = open(SERVER_FIFO, O_RDWR);
    if (server_fd < 0) {
        perror("open server FIFO");
        unlink(SERVER_FIFO);
        exit(1);
    }

    log_message(0, "Server started. Listening on %s", SERVER_FIFO);

    FILE *server_file = fdopen(server_fd, "r");
    if (!server_file) {
        perror("fdopen");
        close(server_fd);
        unlink(SERVER_FIFO);
        exit(1);
    }

    char buffer[MAX_BUFFER];
    while (1) {
        if (fgets(buffer, sizeof(buffer), server_file) == NULL) {
            if (errno == EINTR) continue;
            usleep(100000);
            continue;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        pid_t client_pid;
        char command[64], payload[MAX_BUFFER - 64 - 10];
        if (sscanf(buffer, "%d %63s %[^\n]", &client_pid, command, payload) != 3) {
            log_message(0, "Malformed request: %s", buffer);
            continue;
        }

        log_message(client_pid, "Received request: command=%s payload=%s", command, payload);

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            log_message(client_pid, "Fork failed");
            continue;
        }

        if (pid == 0) {
            fclose(server_file);

            char client_fifo[256];
            snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", client_pid);

            int client_fd = open(client_fifo, O_WRONLY);
            if (client_fd < 0) {
                log_message(client_pid, "Failed to open client FIFO: %s", strerror(errno));
                exit(1);
            }

            char response[MAX_BUFFER];
            process_request(client_pid, command, payload, response, sizeof(response));

            write(client_fd, response, strlen(response));
            write(client_fd, "\n", 1);
            close(client_fd);

            log_message(client_pid, "Response sent: %s", response);
            exit(0);
        }
    }

    fclose(server_file);
    unlink(SERVER_FIFO);
    return 0;
}