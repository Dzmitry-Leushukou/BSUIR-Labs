#include <windows.h>
#include <iostream>
#include <vector>
#include "../ChatCommon.h"

struct ClientInfo
{
    HANDLE hPipe;
};

CRITICAL_SECTION g_clientsLock;
std::vector<ClientInfo> g_clients;

void AddClient(HANDLE hPipe)
{
    EnterCriticalSection(&g_clientsLock);
    ClientInfo info;
    info.hPipe = hPipe;
    g_clients.push_back(info);
    LeaveCriticalSection(&g_clientsLock);
}

void RemoveClient(HANDLE hPipe)
{
    EnterCriticalSection(&g_clientsLock);
    for (auto it = g_clients.begin(); it != g_clients.end(); ++it)
    {
        if (it->hPipe == hPipe)
        {
            g_clients.erase(it);
            break;
        }
    }
    LeaveCriticalSection(&g_clientsLock);
}

void BroadcastMessage(const ChatMessage& msg)
{
    EnterCriticalSection(&g_clientsLock);

    for (auto it = g_clients.begin(); it != g_clients.end(); )
    {
        DWORD bytesWritten = 0;
        BOOL ok = WriteFile(
            it->hPipe,
            &msg,
            sizeof(msg),
            &bytesWritten,
            nullptr
        );

        if (!ok || bytesWritten != sizeof(msg))
        {
            // ТОЛЬКО убираем клиента из рассылки,
            // handle не закрываем здесь
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
    AddClient(hPipe);

    std::wcout << L"Client connected, handle = " << hPipe << std::endl;

    ChatMessage msg{};
    DWORD bytesRead = 0;

    OVERLAPPED ol{};
    ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (!ol.hEvent)
    {
        std::wcerr << L"CreateEvent failed, error = " << GetLastError() << std::endl;
        RemoveClient(hPipe);
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
                        // Клиент нормальным образом отключился
                        break;
                    }
                    std::wcerr << L"GetOverlappedResult failed, error = " << err << std::endl;
                    break;
                }
            }
            else if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA)
            {
                // Клиент закрыл pipe (например, /quit)
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
            // Ничего не прочитали — считаем, что клиент отключился
            break;
        }

        if (bytesRead != sizeof(msg))
        {
            std::wcerr << L"Unexpected message size: " << bytesRead << std::endl;
            continue;
        }

        std::wcout << L"[" << msg.from << L"]: " << msg.text << std::endl;
        BroadcastMessage(msg);
    }

    std::wcout << L"Client disconnected, handle = " << hPipe << std::endl;

    RemoveClient(hPipe);
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    CloseHandle(ol.hEvent);

    return 0;
}

int wmain()
{
    InitializeCriticalSection(&g_clientsLock);

    std::wcout << L"Server started. Waiting for clients..." << std::endl;

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
            break;
        }

        OVERLAPPED ol{};
        ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!ol.hEvent)
        {
            std::wcerr << L"CreateEvent failed, error = " << GetLastError() << std::endl;
            CloseHandle(hPipe);
            break;
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
    }

    DeleteCriticalSection(&g_clientsLock);
    return 0;
}
