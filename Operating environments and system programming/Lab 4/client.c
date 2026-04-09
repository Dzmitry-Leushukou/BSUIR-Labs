#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_FIFO "/tmp/server_fifo"
#define MAX_BUFFER  1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [payload]\n", argv[0]);
        fprintf(stderr, "Commands: ECHO, UPPER, LOWER, TIME, CALC (payload: '2 + 3')\n");
        fprintf(stderr, "If only payload given, command ECHO is assumed.\n");
        exit(1);
    }

    char command[64];
    char payload[MAX_BUFFER];

    if (argc == 2) {
        strcpy(command, "ECHO");
        strcpy(payload, argv[1]);
    } else {
        strncpy(command, argv[1], sizeof(command)-1);
        command[sizeof(command)-1] = '\0';
        strncpy(payload, argv[2], sizeof(payload)-1);
        payload[sizeof(payload)-1] = '\0';
    }

    pid_t pid = getpid();
    char client_fifo[256];
    snprintf(client_fifo, sizeof(client_fifo), "/tmp/client_fifo_%d", pid);

    if (mkfifo(client_fifo, 0666) < 0 && errno != EEXIST) {
        perror("mkfifo client");
        exit(1);
    }

    int client_fd = open(client_fifo, O_RDONLY | O_NONBLOCK);
    if (client_fd < 0) {
        perror("open client FIFO");
        unlink(client_fifo);
        exit(1);
    }

    int server_fd = open(SERVER_FIFO, O_WRONLY);
    if (server_fd < 0) {
        perror("open server FIFO");
        close(client_fd);
        unlink(client_fifo);
        exit(1);
    }

    char request[MAX_BUFFER];
    snprintf(request, sizeof(request), "%d %s %s\n", pid, command, payload);
    if (write(server_fd, request, strlen(request)) < 0) {
        perror("write to server");
        close(server_fd);
        close(client_fd);
        unlink(client_fifo);
        exit(1);
    }
    close(server_fd);

    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags & ~O_NONBLOCK);

    char response[MAX_BUFFER];
    ssize_t n = read(client_fd, response, sizeof(response) - 1);
    if (n > 0) {
        response[n] = '\0';
        printf("Response from server: %s\n", response);
    } else {
        printf("No response or error\n");
    }

    close(client_fd);
    unlink(client_fifo);
    return 0;
}