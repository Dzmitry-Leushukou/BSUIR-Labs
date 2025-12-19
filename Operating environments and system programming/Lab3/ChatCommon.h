#pragma once
#include <windows.h>

struct ChatMessage
{
    wchar_t from[32];
    wchar_t text[256];
    bool isPrivate;      
    wchar_t to[32];      
};

#define CHAT_PIPE_NAME L"\\\\.\\pipe\\ChatPipe"
#define MAX_HISTORY 100  