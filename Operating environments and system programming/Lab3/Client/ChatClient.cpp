#include <windows.h>
#include <iostream>
#include <string>
#include "../ChatCommon.h"

HANDLE g_hPipe = INVALID_HANDLE_VALUE;
std::wstring g_userName;

DWORD WINAPI ReceiverThread(LPVOID)
{
    ChatMessage msg{};
    DWORD bytesRead;
    OVERLAPPED ol{};
    ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    while (true)
    {
        ResetEvent(ol.hEvent);
        BOOL ok = ReadFile(g_hPipe, &msg, sizeof(msg), &bytesRead, &ol);
        if (!ok)
        {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING)
            {
                WaitForSingleObject(ol.hEvent, INFINITE);
                GetOverlappedResult(g_hPipe, &ol, &bytesRead, FALSE);
            }
            else
            {
                std::wcout << L"\nConnection to server lost." << std::endl;
                break;
            }
        }
        if (bytesRead == 0) break;
        std::wcout << L"\n[" << msg.from << L"]: " << msg.text << std::endl;
        std::wcout << L"> " << std::flush;
    }
    CloseHandle(ol.hEvent);
    return 0;
}

bool ConnectToServer()
{
    while (true)
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
            std::wcerr << L"Failed to connect to server. Error: " << err << std::endl;
            return false;
        }

        if (!WaitNamedPipeW(CHAT_PIPE_NAME, 5000))
        {
            std::wcerr << L"Wait for server timed out." << std::endl;
            return false;
        }
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

int wmain()
{
    std::wcout << L"Enter your name: ";
    std::getline(std::wcin, g_userName);
    if (g_userName.empty()) g_userName = L"Anon";
    if (g_userName.size() >= 31) g_userName.resize(31);

    if (!ConnectToServer())
    {
        std::wcerr << L"Connection failed." << std::endl;
        return 1;
    }

    std::wcout << L"Connected to server. Type /quit to exit." << std::endl;
    HANDLE hRecvThread = CreateThread(nullptr, 0, ReceiverThread, nullptr, 0, nullptr);
    if (!hRecvThread)
    {
        std::wcerr << L"Failed to create receiver thread." << std::endl;
        CloseHandle(g_hPipe);
        return 1;
    }

    ChatMessage join{};
    wcsncpy_s(join.from, g_userName.c_str(), _TRUNCATE);
    wcsncpy_s(join.text, L"joined the chat", _TRUNCATE);
    DWORD bytesWritten = 0;
    WriteFile(g_hPipe, &join, sizeof(join), &bytesWritten, nullptr);

    std::wstring line;
    std::wcout << L"> " << std::flush;
    while (std::getline(std::wcin, line))
    {
        if (line == L"/quit") break;
        if (line.empty()) { std::wcout << L"> " << std::flush; continue; }

        ChatMessage msg{};
        wcsncpy_s(msg.from, g_userName.c_str(), _TRUNCATE);
        wcsncpy_s(msg.text, line.c_str(), _TRUNCATE);
        bytesWritten = 0;
        WriteFile(g_hPipe, &msg, sizeof(msg), &bytesWritten, nullptr);
        std::wcout << L"> " << std::flush;
    }

    ChatMessage leave{};
    wcsncpy_s(leave.from, g_userName.c_str(), _TRUNCATE);
    wcsncpy_s(leave.text, L"left the chat", _TRUNCATE);
    WriteFile(g_hPipe, &leave, sizeof(leave), &bytesWritten, nullptr);

    CloseHandle(g_hPipe);
    WaitForSingleObject(hRecvThread, INFINITE);
    CloseHandle(hRecvThread);
    return 0;
}
