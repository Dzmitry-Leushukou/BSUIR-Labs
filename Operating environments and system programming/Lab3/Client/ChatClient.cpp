#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include "../ChatCommon.h"

HANDLE g_hPipe = INVALID_HANDLE_VALUE;
std::wstring g_userName;
bool g_running = true;

DWORD WINAPI ReceiverThread(LPVOID)
{
    ChatMessage msg{};
    DWORD bytesRead;
    OVERLAPPED ol{};
    ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    while (g_running)
    {
        ResetEvent(ol.hEvent);
        BOOL ok = ReadFile(g_hPipe, &msg, sizeof(msg), &bytesRead, &ol);
        if (!ok)
        {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING)
            {
                DWORD waitRes = WaitForSingleObject(ol.hEvent, 100);
                if (waitRes == WAIT_TIMEOUT) continue;
                if (waitRes != WAIT_OBJECT_0) break;

                if (!GetOverlappedResult(g_hPipe, &ol, &bytesRead, FALSE))
                {
                    DWORD err2 = GetLastError();
                    if (err2 == ERROR_BROKEN_PIPE || err2 == ERROR_NO_DATA)
                    {
                        std::wcout << L"\nConnection to server lost." << std::endl;
                        break;
                    }
                    continue;
                }
            }
            else if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA)
            {
                std::wcout << L"\nConnection to server lost." << std::endl;
                break;
            }
            else
            {
                continue;
            }
        }

        if (bytesRead != sizeof(msg)) continue;

        if (msg.isPrivate && wcscmp(msg.to, g_userName.c_str()) == 0)
        {
            std::wcout << L"\n[PRIVATE from " << msg.from << L"]: " << msg.text << std::endl;
        }
        else if (!msg.isPrivate)
        {
            if (wcscmp(msg.from, g_userName.c_str()) != 0 ||
                wcsstr(msg.text, L"joined") != nullptr ||
                wcsstr(msg.text, L"left") != nullptr)
            {
                std::wcout << L"\n[" << msg.from << L"]: " << msg.text << std::endl;
            }
        }

        std::wcout << L"> " << std::flush;
    }

    if (ol.hEvent) CloseHandle(ol.hEvent);
    g_running = false;
    return 0;
}

bool ConnectToServer()
{
    for (int attempt = 0; attempt < 10; ++attempt)
    {
        g_hPipe = CreateFileW(
            CHAT_PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            nullptr
        );

        if (g_hPipe != INVALID_HANDLE_VALUE) break;

        DWORD err = GetLastError();
        if (err != ERROR_PIPE_BUSY)
        {
            if (attempt == 0)
                std::wcerr << L"Failed to connect to server. Error: " << err << std::endl;
            Sleep(1000);
            continue;
        }

        if (!WaitNamedPipeW(CHAT_PIPE_NAME, 5000))
        {
            std::wcerr << L"Wait for server timed out." << std::endl;
            return false;
        }
    }

    if (g_hPipe == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"Failed to connect to server after multiple attempts." << std::endl;
        return false;
    }

    DWORD dwMode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(g_hPipe, &dwMode, nullptr, nullptr))
    {
        std::wcerr << L"SetNamedPipeHandleState failed. Error: " << GetLastError() << std::endl;
        CloseHandle(g_hPipe);
        g_hPipe = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

bool SendMessageToServer(const ChatMessage& msg)
{
    DWORD bytesWritten = 0;
    OVERLAPPED ol{};
    ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    BOOL ok = WriteFile(g_hPipe, &msg, sizeof(msg), &bytesWritten, &ol);

    if (!ok && GetLastError() == ERROR_IO_PENDING)
    {
        DWORD waitRes = WaitForSingleObject(ol.hEvent, 2000);
        if (waitRes != WAIT_OBJECT_0)
        {
            CloseHandle(ol.hEvent);
            return false;
        }

        if (!GetOverlappedResult(g_hPipe, &ol, &bytesWritten, FALSE))
        {
            CloseHandle(ol.hEvent);
            return false;
        }
    }

    if (ol.hEvent) CloseHandle(ol.hEvent);
    return (bytesWritten == sizeof(msg));
}

void ShowHelp()
{
    std::wcout << L"\nAvailable commands:" << std::endl;
    std::wcout << L"  /help - show this help" << std::endl;
    std::wcout << L"  /users - show online users" << std::endl;
    std::wcout << L"  @username message - send private message" << std::endl;
    std::wcout << L"  /quit - disconnect" << std::endl;
    std::wcout << L"> " << std::flush;
}

int wmain()
{
    std::wcout << L"Welcome to Chat Client!" << std::endl;
    std::wcout << L"Enter your name: ";
    std::getline(std::wcin, g_userName);

    if (g_userName.empty()) g_userName = L"Anon";
    if (g_userName.size() >= 31) g_userName.resize(31);

    if (!ConnectToServer())
    {
        std::wcerr << L"Connection failed." << std::endl;
        return 1;
    }

    ChatMessage joinMsg{};
    wcscpy_s(joinMsg.from, g_userName.c_str());
    wcscpy_s(joinMsg.text, L"");
    joinMsg.isPrivate = false;

    if (!SendMessageToServer(joinMsg))
    {
        std::wcerr << L"Failed to send join message." << std::endl;
        CloseHandle(g_hPipe);
        return 1;
    }

    std::wcout << L"\nConnected to server. Type /help for available commands." << std::endl;
    std::wcout << L"Type /quit to exit." << std::endl << std::endl;

    HANDLE hRecvThread = CreateThread(nullptr, 0, ReceiverThread, nullptr, 0, nullptr);
    if (!hRecvThread)
    {
        std::wcerr << L"Failed to create receiver thread." << std::endl;
        CloseHandle(g_hPipe);
        return 1;
    }

    std::wstring line;
    std::wcout << L"> " << std::flush;

    while (g_running && std::getline(std::wcin, line))
    {
        if (line.empty())
        {
            std::wcout << L"> " << std::flush;
            continue;
        }

        if (line == L"/quit" || line == L"/exit")
        {
            break;
        }
        else if (line == L"/help")
        {
            ShowHelp();
            continue;
        }
        else if (line == L"/users")
        {
            ChatMessage msg{};
            wcscpy_s(msg.from, g_userName.c_str());
            wcscpy_s(msg.text, L"/users");
            msg.isPrivate = false;

            SendMessageToServer(msg);
            std::wcout << L"> " << std::flush;
            continue;
        }

        ChatMessage msg{};
        wcscpy_s(msg.from, g_userName.c_str());
        msg.isPrivate = false;

        if (line[0] == L'@')
        {
            size_t spacePos = line.find(L' ');
            if (spacePos != std::wstring::npos)
            {
                std::wstring toUser = line.substr(1, spacePos - 1);
                std::wstring text = line.substr(spacePos + 1);

                if (!toUser.empty() && !text.empty())
                {
                    wcscpy_s(msg.to, toUser.c_str());
                    wcscpy_s(msg.text, text.c_str());
                    msg.isPrivate = true;
                }
            }
        }

        if (!msg.isPrivate)
        {
            wcscpy_s(msg.text, line.c_str());
        }

        if (!SendMessageToServer(msg))
        {
            std::wcout << L"\nFailed to send message. Connection might be lost." << std::endl;
            break;
        }

        if (!msg.isPrivate)
        {
            std::wcout << L"[" << g_userName << L"]: " << line << std::endl;
        }
        else
        {
            std::wcout << L"[PRIVATE to " << msg.to << L"]: " << msg.text << std::endl;
        }

        std::wcout << L"> " << std::flush;
    }

    g_running = false;

    ChatMessage leaveMsg{};
    wcscpy_s(leaveMsg.from, g_userName.c_str());
    wcscpy_s(leaveMsg.text, L"/quit");
    leaveMsg.isPrivate = false;
    SendMessageToServer(leaveMsg);

    Sleep(100);

    CloseHandle(g_hPipe);
    WaitForSingleObject(hRecvThread, 2000);
    CloseHandle(hRecvThread);

    std::wcout << L"\nDisconnected from server." << std::endl;
    return 0;
}