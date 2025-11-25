#pragma once
#include <windows.h>

struct ChatMessage
{
    wchar_t from[32];
    wchar_t text[256];
};

#define CHAT_PIPE_NAME L"\\\\.\\pipe\\ChatPipe"
