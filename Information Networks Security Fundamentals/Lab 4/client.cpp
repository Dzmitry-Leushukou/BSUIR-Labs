#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <csignal>

const int PORT = 8080;
const char* SERVER_IP = "127.0.0.1";
const int BUFFER_SIZE = 4096;

int sock_fd = -1;
volatile bool running = true;

void signal_handler(int) {
    running = false;
    if (sock_fd != -1) {
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
        sock_fd = -1;
    }
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock_fd);
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    std::cout << "Connected to server " << SERVER_IP << ":" << PORT << std::endl;
    std::cout << "Enter messages (empty line to quit):" << std::endl;

    std::string input_buffer; 
    fd_set read_fds;
    int max_fd = std::max(sock_fd, STDIN_FILENO) + 1;

    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(sock_fd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        struct timeval timeout = {1, 0}; 
        int activity = select(max_fd, &read_fds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (!running) break;

        if (FD_ISSET(sock_fd, &read_fds)) {
            char buffer[BUFFER_SIZE];
            ssize_t n = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            if (n > 0) {
                buffer[n] = '\0';
                std::cout << "\nServer: " << buffer << std::flush;
                std::cout << "> " << std::flush; 
            } else if (n == 0) {
                std::cout << "\nServer closed connection.\n";
                break;
            } else {
                perror("recv");
                break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) == 1) {
                if (ch == '\n') {
                    if (input_buffer.empty() || input_buffer == "quit") {
                        std::cout << "Exiting...\n";
                        running = false;
                        break;
                    }
                    std::string to_send = input_buffer + '\n';
                    ssize_t sent = send(sock_fd, to_send.c_str(), to_send.size(), 0);
                    if (sent == -1) {
                        perror("send");
                        running = false;
                        break;
                    }
                    input_buffer.clear();
                    std::cout << "> " << std::flush;
                } else {
                    input_buffer += ch;
                    std::cout << ch << std::flush;
                }
            } else {
                std::cout << "\nEOF detected, exiting.\n";
                break;
            }
        }
    }

    if (sock_fd != -1) {
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
    }

    std::cout << "\nClient terminated." << std::endl;
    return 0;
}