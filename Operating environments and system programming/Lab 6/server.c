#include "common.h"
#include <sys/select.h>
#include <sys/time.h>

static client_t clients[MAX_CLIENTS];
static int client_count = 0;
static int listen_sock = -1;

void cleanup() {
    if (listen_sock != -1) close(listen_sock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != -1) close(clients[i].socket);
    }
}

int find_client_by_name(const char *name) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].name, name) == 0)
            return i;
    }
    return -1;
}

void broadcast_message(const char *msg, int sender_sock) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != sender_sock) {
            if (send(clients[i].socket, msg, strlen(msg), 0) == -1) {
                perror("send");
            }
        }
    }
}

int send_private_message(const char *recipient, const char *msg, int sender_sock) {
    int idx = find_client_by_name(recipient);
    if (idx == -1) return -1;
    if (send(clients[idx].socket, msg, strlen(msg), 0) == -1) return -1;
    return 0;
}

void process_client_message(int client_sock, const char *buffer) {
    int sender_idx = -1;
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client_sock) {
            sender_idx = i;
            break;
        }
    }
    if (sender_idx == -1) return;

    char full_msg[BUFFER_SIZE + NAME_LEN + 10];

    if (buffer[0] == '@') {
        char recipient[NAME_LEN];
        char body[BUFFER_SIZE];
        if (sscanf(buffer, "@%31s %[^\n]", recipient, body) == 2) {
            char private_msg[BUFFER_SIZE + NAME_LEN + 20];
            snprintf(private_msg, sizeof(private_msg), "[PM from %s] %s\n",
                     clients[sender_idx].name, body);
            if (send_private_message(recipient, private_msg, client_sock) == 0) {
                char confirm[BUFFER_SIZE];
                snprintf(confirm, sizeof(confirm), "[System] Message sent to %s\n", recipient);
                send(client_sock, confirm, strlen(confirm), 0);
            } else {
                char error_msg[BUFFER_SIZE];
                snprintf(error_msg, sizeof(error_msg), 
                         "[System] User '%s' not found or offline\n", recipient);
                send(client_sock, error_msg, strlen(error_msg), 0);
            }
            return;
        }
    }

    snprintf(full_msg, sizeof(full_msg), "%s: %s\n", clients[sender_idx].name, buffer);
    broadcast_message(full_msg, client_sock);
}

void remove_client(int client_sock) {
    int i;
    for (i = 0; i < client_count; i++) {
        if (clients[i].socket == client_sock) break;
    }
    if (i == client_count) return;
    
    char leave_msg[BUFFER_SIZE + NAME_LEN];
    snprintf(leave_msg, sizeof(leave_msg), "[System] %s left the chat\n", clients[i].name);
    broadcast_message(leave_msg, client_sock);

    close(clients[i].socket);
    for (int j = i; j < client_count - 1; j++) {
        clients[j] = clients[j+1];
    }
    client_count--;
}

void accept_new_client() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int new_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &addr_len);
    if (new_sock == -1) {
        perror("accept");
        return;
    }
    
    if (client_count >= MAX_CLIENTS) {
        const char *msg = "[System] Server full. Try later.\n";
        send(new_sock, msg, strlen(msg), 0);
        close(new_sock);
        return;
    }

    const char *name_prompt = "[System] Enter your name: ";
    send(new_sock, name_prompt, strlen(name_prompt), 0);
    
    char name[NAME_LEN];
    int bytes = recv(new_sock, name, sizeof(name)-1, 0);
    if (bytes <= 0) {
        close(new_sock);
        return;
    }
    name[bytes] = '\0';
    char *p = strchr(name, '\n');
    if (p) *p = '\0';

    if (find_client_by_name(name) != -1) {
        const char *err_msg = "[System] Name already taken. Disconnected.\n";
        send(new_sock, err_msg, strlen(err_msg), 0);
        close(new_sock);
        return;
    }

    clients[client_count].socket = new_sock;
    strncpy(clients[client_count].name, name, NAME_LEN-1);
    clients[client_count].name[NAME_LEN-1] = '\0';
    clients[client_count].addr = client_addr;
    client_count++;
    
    char join_msg[BUFFER_SIZE + NAME_LEN];
    snprintf(join_msg, sizeof(join_msg), "[System] %s joined the chat\n", name);
    broadcast_message(join_msg, new_sock);
    
    printf("New connection: %s (%s:%d), total clients: %d\n", 
           name, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_count);
}

void run_server() {
    fd_set read_fds;
    int max_fd;
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(listen_sock, &read_fds);
        max_fd = listen_sock;
        
        for (int i = 0; i < client_count; i++) {
            int fd = clients[i].socket;
            FD_SET(fd, &read_fds);
            if (fd > max_fd) max_fd = fd;
        }
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }

        if (FD_ISSET(listen_sock, &read_fds)) {
            accept_new_client();
        }

        for (int i = 0; i < client_count; i++) {
            int fd = clients[i].socket;
            if (FD_ISSET(fd, &read_fds)) {
                char buffer[BUFFER_SIZE];
                int bytes = recv(fd, buffer, sizeof(buffer)-1, 0);
                if (bytes <= 0) {
                    remove_client(fd);
                    i--;
                    continue;
                }
                buffer[bytes] = '\0';
                char *newline = strchr(buffer, '\n');
                if (newline) *newline = '\0';
                process_client_message(fd, buffer);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int yes = 1;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }
    
    listen_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listen_sock == -1) {
        perror("socket");
        freeaddrinfo(res);
        exit(1);
    }
    
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        close(listen_sock);
        freeaddrinfo(res);
        exit(1);
    }
    
    if (bind(listen_sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        close(listen_sock);
        freeaddrinfo(res);
        exit(1);
    }
    
    freeaddrinfo(res);
    
    if (listen(listen_sock, 10) == -1) {
        perror("listen");
        close(listen_sock);
        exit(1);
    }
    
    printf("Chat server started on port %s\n", PORT);
    signal(SIGINT, cleanup);

    run_server();
    
    cleanup();
    return 0;
}