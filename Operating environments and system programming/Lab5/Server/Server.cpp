#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")

struct ClientInfo {
    SOCKET socket;
    std::string name;
};

std::vector<ClientInfo> g_clients;
std::mutex g_clientsMutex;

void BroadcastMessage(const std::string& msg, SOCKET sender, const std::string& toName) {
    std::lock_guard<std::mutex> lock(g_clientsMutex);
    for (auto& client : g_clients) {
        if (toName == "*") {
            if (client.socket == sender) continue;
            send(client.socket, msg.c_str(), (int)msg.size(), 0);
        }
        else {
            if (client.name == toName) {
                send(client.socket, msg.c_str(), (int)msg.size(), 0);
                break;
            }
        }
    }
}

void RemoveClient(SOCKET s) {
    std::lock_guard<std::mutex> lock(g_clientsMutex);
    for (auto it = g_clients.begin(); it != g_clients.end(); ++it) {
        if (it->socket == s) {
            closesocket(it->socket);
            g_clients.erase(it);
            break;
        }
    }
}

void ClientThread(SOCKET clientSocket) {
    char buf[4096];

    int bytes = recv(clientSocket, buf, sizeof(buf) - 1, 0);
    if (bytes <= 0) {
        RemoveClient(clientSocket);
        return;
    }
    buf[bytes] = '\0';
    std::string firstMsg(buf);

    std::string clientName;
    const std::string namePrefix = "NAME:";

    if (firstMsg.rfind(namePrefix, 0) == 0) {
        clientName = firstMsg.substr(namePrefix.size());
        while (!clientName.empty() && (clientName.back() == '\n' || clientName.back() == '\r'))
            clientName.pop_back();
    }
    else {
        clientName = "Unknown";
    }

    {
        std::lock_guard<std::mutex> lock(g_clientsMutex);
        g_clients.push_back({ clientSocket, clientName });
    }

    std::cout << "Client connected: " << clientName << std::endl;

    {
        std::string joinMsg = "[Server] " + clientName + " joined the chat.\n";
        BroadcastMessage(joinMsg, INVALID_SOCKET, "*");
    }

    while (true) {
        bytes = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        if (bytes <= 0) {
            std::cout << "Client disconnected: " << clientName << std::endl;
            break;
        }

        buf[bytes] = '\0';
        std::string received(buf);

        std::string toName = "*";
        std::string text = received;

        size_t colonPos = received.find(':');
        if (colonPos != std::string::npos) {
            toName = received.substr(0, colonPos);
            text = received.substr(colonPos + 1);
        }

        while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
            text.pop_back();

        if (text == "/quit") {
            std::cout << "Client quit: " << clientName << std::endl;
            break;
        }

        std::string finalMsg;
        if (toName == "*")
            finalMsg = "[" + clientName + "] " + text + "\n";
        else
            finalMsg = "[" + clientName + " -> " + toName + "] " + text + "\n";

        std::cout << "Message from " << clientName << " to " << toName
            << ": " << text << std::endl;

        BroadcastMessage(finalMsg, clientSocket, toName);
    }

    {
        std::string leaveMsg = "[Server] " + clientName + " left the chat.\n";
        BroadcastMessage(leaveMsg, INVALID_SOCKET, "*");
    }

    RemoveClient(clientSocket);
}

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cout << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cout << "socket() failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started. Port: 54000" << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        int clientAddrSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cout << "accept() failed: " << WSAGetLastError() << std::endl;
            break;
        }

        std::thread t(ClientThread, clientSocket);
        t.detach();
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
