#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const int PORT = 8080;
const char* SERVER_IP = "127.0.0.1";
const int BUFFER_SIZE = 4096;

std::atomic<bool> running(true);
int sock_fd = -1;

void signal_handler(int) {
    running = false;
    if (sock_fd != -1) {
        shutdown(sock_fd, SHUT_RDWR);
        close(sock_fd);
    }
}

void receive_thread_func() {
    char buffer[BUFFER_SIZE];
    while (running) {
        ssize_t n = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "Server: " << buffer << std::flush;
        } else if (n == 0) {
            std::cout << "Server closed connection.\n";
            running = false;
            break;
        } else {
            if (errno != EINTR) {
                perror("recv");
                running = false;
            }
            break;
        }
    }
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Создание сокета
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock_fd);
        return 1;
    }

    // Подключение
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    std::cout << "Connected to server " << SERVER_IP << ":" << PORT << std::endl;
    std::cout << "Enter messages (empty line to quit):" << std::endl;

    // Запуск потока приёма сообщений
    std::thread receive_thread(receive_thread_func);

    // Основной цикл отправки сообщений
    std::string line;
    while (running) {
        if (!std::getline(std::cin, line)) {
            // EOF (Ctrl+D)
            running = false;
            break;
        }

        if (line.empty() || line == "quit") {
            running = false;
            break;
        }

        line += '\n'; // добавляем разделитель
        ssize_t sent = send(sock_fd, line.c_str(), line.size(), 0);
        if (sent == -1) {
            perror("send");
            running = false;
            break;
        }
    }

    // Корректное завершение
    if (sock_fd != -1) {
        shutdown(sock_fd, SHUT_WR); // сообщаем серверу, что больше ничего не отправим
        // Даём потоку приёма время получить остаток данных
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        close(sock_fd);
    }

    if (receive_thread.joinable())
        receive_thread.join();

    std::cout << "Client terminated." << std::endl;
    return 0;
}