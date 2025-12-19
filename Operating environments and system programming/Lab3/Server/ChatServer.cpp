#include <windows.h>
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include "../ChatCommon.h"

struct ClientInfo
{
    HANDLE hPipe;
    wchar_t name[32];
    bool active;
};

CRITICAL_SECTION g_clientsLock;
std::vector<ClientInfo> g_clients;
std::deque<ChatMessage> g_messageHistory;

void AddClient(HANDLE hPipe, const wchar_t* name)
{
    EnterCriticalSection(&g_clientsLock);

    for (auto& client : g_clients) {
        if (wcscmp(client.name, name) == 0 && client.active) {
            wchar_t newName[32];
            swprintf_s(newName, L"%s_%d", name, GetCurrentThreadId());
            wcscpy_s(client.name, newName);
            break;
        }
    }

    ClientInfo info;
    info.hPipe = hPipe;
    wcscpy_s(info.name, name);
    info.active = true;
    g_clients.push_back(info);

    for (const auto& msg : g_messageHistory) {
        DWORD bytesWritten = 0;
        WriteFile(
            hPipe,
            &msg,
            sizeof(msg),
            &bytesWritten,
            nullptr
        );
    }

    LeaveCriticalSection(&g_clientsLock);
}

void RemoveClient(HANDLE hPipe)
{
    EnterCriticalSection(&g_clientsLock);
    for (auto it = g_clients.begin(); it != g_clients.end(); ++it)
    {
        if (it->hPipe == hPipe)
        {
            it->active = false;
            break;
        }
    }
    LeaveCriticalSection(&g_clientsLock);
}

void BroadcastMessage(const ChatMessage& msg)
{
    EnterCriticalSection(&g_clientsLock);

    for (auto& client : g_clients)
    {
        if (!client.active) continue;

        if (msg.isPrivate) {
            if (wcscmp(client.name, msg.to) != 0 &&
                wcscmp(client.name, msg.from) != 0) {
                continue; 
            }
        }

        DWORD bytesWritten = 0;
        BOOL ok = WriteFile(
            client.hPipe,
            &msg,
            sizeof(msg),
            &bytesWritten,
            nullptr
        );

        if (!ok || bytesWritten != sizeof(msg))
        {
            client.active = false;
        }
    }

    LeaveCriticalSection(&g_clientsLock);
}

void StoreMessage(const ChatMessage& msg)
{
    EnterCriticalSection(&g_clientsLock);
    g_messageHistory.push_back(msg);

    if (g_messageHistory.size() > MAX_HISTORY) {
        g_messageHistory.pop_front();
    }
    LeaveCriticalSection(&g_clientsLock);
}

void CleanupInactiveClients()
{
    EnterCriticalSection(&g_clientsLock);

    for (auto it = g_clients.begin(); it != g_clients.end(); )
    {
        if (!it->active)
        {
            FlushFileBuffers(it->hPipe);
            DisconnectNamedPipe(it->hPipe);
            CloseHandle(it->hPipe);
            it = g_clients.erase(it);
        }
        else
        {
            ++it;
        }
    }

    LeaveCriticalSection(&g_clientsLock);
}

DWORD WINAPI ClientThread(LPVOID param)
{
    HANDLE hPipe = (HANDLE)param;
    wchar_t clientName[32] = { 0 };
    bool nameReceived = false;

    ChatMessage msg{};
    DWORD bytesRead = 0;

    OVERLAPPED ol{};
    ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (!ol.hEvent)
    {
        std::wcerr << L"CreateEvent failed, error = " << GetLastError() << std::endl;
        FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
        return 0;
    }

    while (true)
    {
        ZeroMemory(&msg, sizeof(msg));
        bytesRead = 0;

        ResetEvent(ol.hEvent);

        BOOL ok = ReadFile(
            hPipe,
            &msg,
            sizeof(msg),
            &bytesRead,
            &ol
        );

        if (!ok)
        {
            DWORD err = GetLastError();

            if (err == ERROR_IO_PENDING)
            {
                DWORD waitRes = WaitForSingleObject(ol.hEvent, INFINITE);
                if (waitRes != WAIT_OBJECT_0)
                {
                    std::wcerr << L"WaitForSingleObject failed, error = " << GetLastError() << std::endl;
                    break;
                }

                if (!GetOverlappedResult(hPipe, &ol, &bytesRead, FALSE))
                {
                    err = GetLastError();
                    if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA)
                    {
                        break;
                    }
                    std::wcerr << L"GetOverlappedResult failed, error = " << err << std::endl;
                    break;
                }
            }
            else if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA)
            {
                break;
            }
            else
            {
                std::wcerr << L"ReadFile failed, error = " << err << std::endl;
                break;
            }
        }

        if (bytesRead == 0)
        {
            break;
        }

        if (bytesRead != sizeof(msg))
        {
            std::wcerr << L"Unexpected message size: " << bytesRead << std::endl;
            continue;
        }

        if (!nameReceived)
        {
            wcscpy_s(clientName, msg.from);
            AddClient(hPipe, clientName);
            nameReceived = true;

            ChatMessage joinMsg{};
            wcscpy_s(joinMsg.from, L"System");
            wcsncpy_s(joinMsg.text, (std::wstring(clientName) + L" joined the chat").c_str(), _TRUNCATE);

            std::wcout << L"[" << joinMsg.from << L"]: " << joinMsg.text << std::endl;
            StoreMessage(joinMsg);
            BroadcastMessage(joinMsg);
            continue;
        }

        if (msg.isPrivate && wcscmp(clientName, msg.from) == 0)
        {
            std::wcout << L"[PRIVATE from " << msg.from << L" to " << msg.to << L"]: " << msg.text << std::endl;
        }
        else if (!msg.isPrivate)
        {
            std::wcout << L"[" << msg.from << L"]: " << msg.text << std::endl;
        }

        StoreMessage(msg);
        BroadcastMessage(msg);

        CleanupInactiveClients();
    }

    std::wcout << L"Client disconnected: " << clientName << std::endl;

    ChatMessage leaveMsg{};
    wcscpy_s(leaveMsg.from, L"System");
    wcsncpy_s(leaveMsg.text, (std::wstring(clientName) + L" left the chat").c_str(), _TRUNCATE);

    StoreMessage(leaveMsg);
    BroadcastMessage(leaveMsg);

    RemoveClient(hPipe);
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    CloseHandle(ol.hEvent);

    CleanupInactiveClients();

    return 0;
}

int wmain()
{
    InitializeCriticalSection(&g_clientsLock);

    std::wcout << L"Chat Server started. Waiting for clients..." << std::endl;
    std::wcout << L"Supported commands:" << std::endl;
    std::wcout << L"  /help - show this help" << std::endl;
    std::wcout << L"  /users - show online users" << std::endl;
    std::wcout << L"  @username message - send private message" << std::endl;
    std::wcout << L"  /quit - disconnect" << std::endl << std::endl;

    while (true)
    {
        HANDLE hPipe = CreateNamedPipeW(
            CHAT_PIPE_NAME,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(ChatMessage),
            sizeof(ChatMessage),
            0,
            nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            std::wcerr << L"CreateNamedPipeW failed, error = " << GetLastError() << std::endl;
            Sleep(1000);
            continue;
        }

        OVERLAPPED ol{};
        ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!ol.hEvent)
        {
            std::wcerr << L"CreateEvent failed, error = " << GetLastError() << std::endl;
            CloseHandle(hPipe);
            Sleep(1000);
            continue;
        }

        BOOL connected = ConnectNamedPipe(hPipe, &ol);

        if (!connected)
        {
            DWORD err = GetLastError();

            if (err == ERROR_IO_PENDING)
            {
                DWORD waitRes = WaitForSingleObject(ol.hEvent, INFINITE);
                if (waitRes != WAIT_OBJECT_0)
                {
                    std::wcerr << L"WaitForSingleObject (connect) failed, error = " << GetLastError() << std::endl;
                    CloseHandle(hPipe);
                    CloseHandle(ol.hEvent);
                    continue;
                }
                connected = TRUE;
            }
            else if (err == ERROR_PIPE_CONNECTED)
            {
                connected = TRUE;
            }
            else
            {
                std::wcerr << L"ConnectNamedPipe failed, error = " << err << std::endl;
                CloseHandle(hPipe);
                CloseHandle(ol.hEvent);
                Sleep(1000);
                continue;
            }
        }

        CloseHandle(ol.hEvent);

        if (connected)
        {
            HANDLE hThread = CreateThread(
                nullptr,
                0,
                ClientThread,
                hPipe,
                0,
                nullptr
            );

            if (hThread)
            {
                CloseHandle(hThread);
            }
            else
            {
                std::wcerr << L"CreateThread failed, error = " << GetLastError() << std::endl;
                FlushFileBuffers(hPipe);
                DisconnectNamedPipe(hPipe);
                CloseHandle(hPipe);
            }
        }
        else
        {
            FlushFileBuffers(hPipe);
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
        }

        CleanupInactiveClients();
        Sleep(100); 
    }

    DeleteCriticalSection(&g_clientsLock);
    return 0;
}