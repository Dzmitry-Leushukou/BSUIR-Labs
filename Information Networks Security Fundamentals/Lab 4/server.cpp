#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>

const int PORT = 8080;
const int MAX_EVENTS = 64;
const int MAX_CONNECTIONS = 1024;          // Максимум одновременных соединений
const int CONNECTION_TIMEOUT_SEC = 60;     // Общий таймаут соединения (сек)
const int READ_TIMEOUT_SEC = 10;            // Таймаут на чтение данных (сек)
const int MAX_REQUEST_SIZE = 4096;          // Максимальный размер запроса (байт)
const int MAX_CONNECTIONS_PER_IP = 10;      // Максимум соединений с одного IP

struct Connection {
    int fd;
    std::string ip;
    std::chrono::steady_clock::time_point last_activity;
    std::string buffer;
    bool reading_done;
};

class Server {
private:
    int listen_fd;
    int epoll_fd;
    std::unordered_map<int, Connection*> connections;
    std::unordered_map<std::string, int> ip_connections;
    bool running;

    void set_nonblocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) {
            perror("fcntl F_GETFL");
            return;
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl F_SETFL");
        }
    }

    void add_connection(int client_fd, const std::string& client_ip) {
        auto* conn = new Connection{client_fd, client_ip,
                                    std::chrono::steady_clock::now(),
                                    "", false};
        connections[client_fd] = conn;
        ip_connections[client_ip]++;

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
        ev.data.fd = client_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
            perror("epoll_ctl add client");
            close(client_fd);
            connections.erase(client_fd);
            delete conn;
            ip_connections[client_ip]--;
            if (ip_connections[client_ip] == 0) ip_connections.erase(client_ip);
        }
    }

    void remove_connection(int fd) {
        auto it = connections.find(fd);
        if (it != connections.end()) {
            Connection* conn = it->second;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
            close(fd);
            auto ip_it = ip_connections.find(conn->ip);
            if (ip_it != ip_connections.end()) {
                ip_it->second--;
                if (ip_it->second == 0) ip_connections.erase(ip_it);
            }
            delete conn;
            connections.erase(it);
        }
    }

    void handle_client(int fd) {
        auto it = connections.find(fd);
        if (it == connections.end()) return;
        Connection* conn = it->second;

        // Проверка таймаута чтения
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - conn->last_activity).count();
        if (elapsed > READ_TIMEOUT_SEC) {
            std::cout << "Read timeout for fd " << fd << std::endl;
            remove_connection(fd);
            return;
        }

        char buf[1024];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n > 0) {
            conn->last_activity = now;
            conn->buffer.append(buf, n);

            // Защита от слишком большого запроса
            if (conn->buffer.size() > MAX_REQUEST_SIZE) {
                std::string response = "ERROR: Request too large\n";
                send(fd, response.c_str(), response.size(), 0);
                remove_connection(fd);
                return;
            }

            // Обработка полных строк (протокол: строки разделены \n)
            size_t pos;
            while ((pos = conn->buffer.find('\n')) != std::string::npos) {
                std::string line = conn->buffer.substr(0, pos);
                conn->buffer.erase(0, pos + 1);
                std::cout << "Received from " << conn->ip << ": " << line << std::endl;
                std::string response = "OK: " + line + "\n";
                send(fd, response.c_str(), response.size(), 0);
            }
        } else if (n == 0) {
            std::cout << "Connection closed by client fd " << fd << std::endl;
            remove_connection(fd);
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recv error");
                remove_connection(fd);
            }
        }
    }

    void check_timeouts() {
        auto now = std::chrono::steady_clock::now();
        for (auto it = connections.begin(); it != connections.end(); ) {
            Connection* conn = it->second;
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - conn->last_activity).count();
            if (elapsed > CONNECTION_TIMEOUT_SEC) {
                std::cout << "Connection timeout for fd " << conn->fd << std::endl;
                int fd = conn->fd;
                ++it;
                remove_connection(fd);
            } else {
                ++it;
            }
        }
    }

public:
    Server() : listen_fd(-1), epoll_fd(-1), running(false) {}

    bool init() {
        // Игнорируем SIGPIPE, чтобы запись в закрытый сокет не убивала процесс
        signal(SIGPIPE, SIG_IGN);

        // Ограничение числа открытых файловых дескрипторов
        struct rlimit rl;
        rl.rlim_cur = MAX_CONNECTIONS + 10;
        rl.rlim_max = MAX_CONNECTIONS + 10;
        setrlimit(RLIMIT_NOFILE, &rl); // не критично, если не получится

        listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (listen_fd == -1) {
            perror("socket");
            return false;
        }

        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // TCP_DEFER_ACCEPT – не создавать соединение до получения первых данных
        setsockopt(listen_fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &opt, sizeof(opt));

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind");
            close(listen_fd);
            return false;
        }

        // Большой backlog для очереди соединений
        if (listen(listen_fd, SOMAXCONN) == -1) {
            perror("listen");
            close(listen_fd);
            return false;
        }

        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            perror("epoll_create1");
            close(listen_fd);
            return false;
        }

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = listen_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
            perror("epoll_ctl listen");
            close(listen_fd);
            close(epoll_fd);
            return false;
        }

        running = true;
        return true;
    }

    void run() {
        if (!running) return;

        struct epoll_event events[MAX_EVENTS];
        while (running) {
            int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000); // таймаут 1 с для проверки таймаутов
            if (nfds == -1) {
                if (errno == EINTR) continue;
                perror("epoll_wait");
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                int fd = events[i].data.fd;
                if (fd == listen_fd) {
                    // Принимаем все готовые соединения
                    while (true) {
                        struct sockaddr_in client_addr;
                        socklen_t client_len = sizeof(client_addr);
                        int client_fd = accept4(listen_fd, (struct sockaddr*)&client_addr,
                                                &client_len, SOCK_NONBLOCK);
                        if (client_fd == -1) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                            perror("accept");
                            break;
                        }

                        // Проверка лимита общего числа соединений
                        if (connections.size() >= MAX_CONNECTIONS) {
                            std::cout << "Too many connections, rejecting.\n";
                            close(client_fd);
                            continue;
                        }

                        char ip_str[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
                        std::string client_ip(ip_str);

                        // Проверка лимита соединений с одного IP
                        int count = ip_connections[client_ip]; // 0, если ключа нет
                        if (count >= MAX_CONNECTIONS_PER_IP) {
                            std::cout << "Too many connections from IP " << client_ip << ", rejecting.\n";
                            close(client_fd);
                            continue;
                        }

                        add_connection(client_fd, client_ip);
                        std::cout << "New connection from " << client_ip << " fd=" << client_fd << std::endl;
                    }
                } else {
                    if (events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {
                        std::cout << "Connection error/hangup on fd " << fd << std::endl;
                        remove_connection(fd);
                    } else if (events[i].events & EPOLLIN) {
                        handle_client(fd);
                    }
                }
            }

            check_timeouts();
        }
    }

    void stop() {
        running = false;
        for (auto& [fd, conn] : connections) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
            close(fd);
            delete conn;
        }
        connections.clear();
        ip_connections.clear();
        if (listen_fd != -1) close(listen_fd);
        if (epoll_fd != -1) close(epoll_fd);
        listen_fd = epoll_fd = -1;
    }

    ~Server() { stop(); }
};

int main() {
    Server server;
    if (!server.init()) {
        std::cerr << "Server initialization failed.\n";
        return 1;
    }
    std::cout << "Server started on port " << PORT << std::endl;
    server.run();
    return 0;
}