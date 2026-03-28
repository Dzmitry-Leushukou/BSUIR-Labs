#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>



#define PORT 8080
#define SERVER_IP "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("invalid address / address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        return -1;
    }

        
    std::string payload;
    std::cout << "Enter data to send: ";
    std::getline(std::cin, payload);

    std::cout << "Sending " << payload.size() << " bytes of data...\n";
    send(sock, payload.c_str(), payload.size(), 0);

    ssize_t bytes_read = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::cout << "Server response: " << buffer << "\n";
    }

    close(sock);
    return 0;
}