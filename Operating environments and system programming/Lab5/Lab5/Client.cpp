#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

void RecvThread(SOCKET sock) {
    char buf[4096];
    while (true) {
        int bytes = recv(sock, buf, sizeof(buf) - 1, 0);
        if (bytes <= 0) {
            std::cout << "Connection to server lost." << std::endl;
            break;
        }
        buf[bytes] = '\0';
        std::cout << buf;
    }
}

int main() {
    std::string serverIp = "127.0.0.1";
    std::cout << "Enter server IP (default 127.0.0.1): ";
    std::string inputIp;
    std::getline(std::cin, inputIp);
    if (!inputIp.empty())
        serverIp = inputIp;

    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cout << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cout << "socket() failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());

    if (serverAddr.sin_addr.s_addr == INADDR_NONE) {
        std::cout << "Invalid IP address." << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "connect() failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    std::string nameMsg = "NAME:" + name + "\n";
    send(sock, nameMsg.c_str(), (int)nameMsg.size(), 0);

    std::thread recvThread(RecvThread, sock);
    recvThread.detach();

    std::cout << "Message format: target:message\n";
    std::cout << "target = * to send to everyone. Example: *:Hello all\n";
    std::cout << "To exit type /quit as the message text (for example: *:/quit)\n\n";

    while (true) {
        std::string line;
        std::getline(std::cin, line);

        if (line.empty())
            continue;

        line += "\n";
        int bytesSent = send(sock, line.c_str(), (int)line.size(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cout << "send() failed: " << WSAGetLastError() << std::endl;
            break;
        }

        if (line.find("/quit") != std::string::npos) {
            break;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
