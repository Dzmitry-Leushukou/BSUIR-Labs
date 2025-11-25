#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <queue>
#include <cstring>
#include <cstdio>

// shared buffer configuration
const int BLOCK_COUNT = 8;
const int BLOCK_SIZE = 128;
const int PRODUCER_COUNT = 2;
const int CONSUMER_COUNT = 3;
const int ITEMS_PER_PRODUCER = 50;

// one memory block
struct Block
{
    char data[BLOCK_SIZE];
};

// shared blocks array
Block g_blocks[BLOCK_COUNT];

// semaphores: free blocks / full blocks
HANDLE g_freeBlocks = NULL;
HANDLE g_fullBlocks = NULL;

// critical sections: queues / console output
CRITICAL_SECTION g_queueCS;
CRITICAL_SECTION g_ioCS;

// queues of block indices
std::queue<int> g_freeQueue;
std::queue<int> g_fullQueue;

// simple statistics
volatile LONG g_itemsProduced = 0;
volatile LONG g_itemsConsumed = 0;
volatile LONG g_producersFinished = 0;

// thread parameter: just id
struct ThreadParam
{
    int id;
};

// producer thread: writes data to free blocks
DWORD WINAPI ProducerThread(LPVOID lpParam)
{
    ThreadParam* p = static_cast<ThreadParam*>(lpParam);
    int id = p->id;

    for (int item = 0; item < ITEMS_PER_PRODUCER; ++item)
    {
        WaitForSingleObject(g_freeBlocks, INFINITE);

        EnterCriticalSection(&g_queueCS);
        int idx = g_freeQueue.front();
        g_freeQueue.pop();
        LeaveCriticalSection(&g_queueCS);

        _snprintf_s(g_blocks[idx].data, BLOCK_SIZE, _TRUNCATE, "P%d: item %d", id, item);

        InterlockedIncrement(&g_itemsProduced);

        EnterCriticalSection(&g_queueCS);
        g_fullQueue.push(idx);
        LeaveCriticalSection(&g_queueCS);

        ReleaseSemaphore(g_fullBlocks, 1, NULL);

        EnterCriticalSection(&g_ioCS);
        std::cout << "[P" << id << "] -> block " << idx << " (item " << item << ")\n";
        LeaveCriticalSection(&g_ioCS);

        Sleep(10 + id * 3);
    }

    return 0;
}

// consumer thread: reads data from full blocks
DWORD WINAPI ConsumerThread(LPVOID lpParam)
{
    ThreadParam* p = static_cast<ThreadParam*>(lpParam);
    int id = p->id;

    while (true)
    {
        DWORD res = WaitForSingleObject(g_fullBlocks, 100);
        if (res == WAIT_TIMEOUT)
        {
            if (InterlockedCompareExchange(&g_producersFinished, 0, 0) != 0)
            {
                EnterCriticalSection(&g_queueCS);
                bool empty = g_fullQueue.empty();
                LeaveCriticalSection(&g_queueCS);
                if (empty)
                    break;
            }
            continue;
        }
        else if (res != WAIT_OBJECT_0)
        {
            break;
        }

        EnterCriticalSection(&g_queueCS);
        if (g_fullQueue.empty())
        {
            LeaveCriticalSection(&g_queueCS);
            continue;
        }
        int idx = g_fullQueue.front();
        g_fullQueue.pop();
        LeaveCriticalSection(&g_queueCS);

        char localBuffer[BLOCK_SIZE];
        memcpy(localBuffer, g_blocks[idx].data, BLOCK_SIZE);

        LONG consumed = InterlockedIncrement(&g_itemsConsumed);

        EnterCriticalSection(&g_ioCS);
        std::cout << "    [C" << id << "] <- block " << idx
            << " : \"" << localBuffer << "\""
            << " (total consumed = " << consumed << ")\n";
        LeaveCriticalSection(&g_ioCS);

        EnterCriticalSection(&g_queueCS);
        g_freeQueue.push(idx);
        LeaveCriticalSection(&g_queueCS);

        ReleaseSemaphore(g_freeBlocks, 1, NULL);

        Sleep(15 + id * 2);
    }

    return 0;
}

// monitor thread: prints buffer state periodically
DWORD WINAPI MonitorThread(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    while (true)
    {
        Sleep(500);

        EnterCriticalSection(&g_queueCS);
        int freeCount = static_cast<int>(g_freeQueue.size());
        int fullCount = static_cast<int>(g_fullQueue.size());
        bool finished = (InterlockedCompareExchange(&g_producersFinished, 0, 0) != 0);
        bool emptyFull = g_fullQueue.empty();
        LeaveCriticalSection(&g_queueCS);

        LONG produced = g_itemsProduced;
        LONG consumed = g_itemsConsumed;

        EnterCriticalSection(&g_ioCS);
        std::cout << "[STAT] free=" << freeCount
            << " full=" << fullCount
            << " produced=" << produced
            << " consumed=" << consumed << std::endl;
        LeaveCriticalSection(&g_ioCS);

        if (finished && emptyFull)
            break;
    }

    return 0;
}

int main()
{
    std::cout << "Parallel shared memory processing demo\n";

    InitializeCriticalSection(&g_queueCS);
    InitializeCriticalSection(&g_ioCS);

    for (int i = 0; i < BLOCK_COUNT; ++i)
        g_freeQueue.push(i);

    g_freeBlocks = CreateSemaphore(NULL, BLOCK_COUNT, BLOCK_COUNT, NULL);
    g_fullBlocks = CreateSemaphore(NULL, 0, BLOCK_COUNT, NULL);

    if (!g_freeBlocks || !g_fullBlocks)
    {
        std::cerr << "CreateSemaphore failed, err = " << GetLastError() << "\n";
        return 1;
    }

    HANDLE producerThreads[PRODUCER_COUNT];
    HANDLE consumerThreads[CONSUMER_COUNT];
    ThreadParam producerParams[PRODUCER_COUNT];
    ThreadParam consumerParams[CONSUMER_COUNT];

    for (int i = 0; i < PRODUCER_COUNT; ++i)
    {
        producerParams[i].id = i;
        producerThreads[i] = CreateThread(NULL, 0, ProducerThread, &producerParams[i], 0, NULL);
    }

    for (int i = 0; i < CONSUMER_COUNT; ++i)
    {
        consumerParams[i].id = i;
        consumerThreads[i] = CreateThread(NULL, 0, ConsumerThread, &consumerParams[i], 0, NULL);
    }

    HANDLE monitorThread = CreateThread(NULL, 0, MonitorThread, NULL, 0, NULL);

    DWORD start = GetTickCount();

    WaitForMultipleObjects(PRODUCER_COUNT, producerThreads, TRUE, INFINITE);
    InterlockedExchange(&g_producersFinished, 1);

    WaitForMultipleObjects(CONSUMER_COUNT, consumerThreads, TRUE, INFINITE);
    WaitForSingleObject(monitorThread, INFINITE);

    DWORD end = GetTickCount();

    for (int i = 0; i < PRODUCER_COUNT; ++i)
        CloseHandle(producerThreads[i]);
    for (int i = 0; i < CONSUMER_COUNT; ++i)
        CloseHandle(consumerThreads[i]);
    CloseHandle(monitorThread);

    CloseHandle(g_freeBlocks);
    CloseHandle(g_fullBlocks);

    DeleteCriticalSection(&g_queueCS);
    DeleteCriticalSection(&g_ioCS);

    std::cout << "\nProduced: " << g_itemsProduced
        << ", consumed: " << g_itemsConsumed << "\n";
    std::cout << "Total time: " << (end - start) << " ms\n";
    std::cout << "Press Enter...\n";
    std::cin.get();
    return 0;
}
