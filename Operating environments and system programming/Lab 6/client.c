#include "common.h"
#include <pthread.h>

static int sock = -1;
static char my_name[NAME_LEN];
static volatile int running = 1;

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    int bytes;
    while (running) {
        bytes = recv(sock, buffer, sizeof(buffer)-1, 0);
        if (bytes <= 0) {
            if (running) printf("\n[System] Disconnected from server.\n");
            break;
        }
        buffer[bytes] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    running = 0;
    return NULL;
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    pthread_t recv_thread;
    
    printf("=== Chat Client ===\n");
    printf("Enter your name: ");
    fflush(stdout);
    if (!fgets(my_name, NAME_LEN, stdin)) {
        fprintf(stderr, "Error reading name\n");
        exit(1);
    }
    char *p = strchr(my_name, '\n');
    if (p) *p = '\0';

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char *server_host = "127.0.0.1";
    if (argc >= 2) server_host = argv[1];
    
    if (getaddrinfo(server_host, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }
    
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        freeaddrinfo(res);
        exit(1);
    }
    
    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        close(sock);
        freeaddrinfo(res);
        exit(1);
    }
    
    freeaddrinfo(res);
    printf("Connected to server %s:%s\n", server_host, PORT);

    char prompt[BUFFER_SIZE];
    int bytes = recv(sock, prompt, sizeof(prompt)-1, 0);
    if (bytes <= 0) {
        fprintf(stderr, "Server closed connection\n");
        close(sock);
        exit(1);
    }
    prompt[bytes] = '\0';
    printf("%s", prompt);
    fflush(stdout);

    send(sock, my_name, strlen(my_name), 0);

    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("pthread_create");
        close(sock);
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    while (running) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            break;
        }
    }

    running = 0;
    close(sock);
    pthread_join(recv_thread, NULL);
    return 0;
}