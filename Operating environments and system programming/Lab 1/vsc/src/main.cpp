#define NOMINMAX
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <random>
#include <ctime>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <psapi.h>
#include <algorithm>

std::mt19937_64 gen(time(NULL));
std::vector<int> test;

ULONG64 GetProcessCpuTime() {
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime)) {
        ULARGE_INTEGER kt, ut;
        kt.LowPart = kernelTime.dwLowDateTime;
        kt.HighPart = kernelTime.dwHighDateTime;
        ut.LowPart = userTime.dwLowDateTime;
        ut.HighPart = userTime.dwHighDateTime;
        return (kt.QuadPart + ut.QuadPart) / 10000;
    }
    return 0;
}

ULONG64 GetThreadCpuTime(DWORD threadId) {
    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, threadId);
    if (hThread) {
        FILETIME createTime, exitTime, kernelTime, userTime;
        if (GetThreadTimes(hThread, &createTime, &exitTime, &kernelTime, &userTime)) {
            ULARGE_INTEGER kt, ut;
            kt.LowPart = kernelTime.dwLowDateTime;
            kt.HighPart = kernelTime.dwHighDateTime;
            ut.LowPart = userTime.dwLowDateTime;
            ut.HighPart = userTime.dwHighDateTime;
            CloseHandle(hThread);
            return (kt.QuadPart + ut.QuadPart) / 10000;
        }
        CloseHandle(hThread);
    }
    return 0;
}

int getInt(int& data, bool clr = false, int l = INT32_MIN, int r = INT32_MAX, std::string text = "") {
    if (clr) system("cls");
    std::cout << "Write number (" << l << "..." << r << "). " << text << ": ";
    std::string s;
    getline(std::cin, s);
    try {
        data = std::stoi(s);
        if (data < l || data > r) throw std::invalid_argument("");
    }
    catch (...) {
        std::cout << "Wrong input. Check limits and try again\n";
        return getInt(data, clr, l, r, text);
    }
    return data;
}

struct settings {
    int threads = 3;
    int arraySize = 1000000;
    bool randomValue = true;
    bool showValue = false;
    int type_of_sort = 0;
} config;

struct ThreadStatus {
    std::atomic<bool> done{ false };
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    DWORD thread_id{ 0 };
    std::atomic<double> cpu_usage{ 0.0 };
    ULONG64 total_cpu_time{ 0 };

    ThreadStatus() = default;

    ThreadStatus(const ThreadStatus& other) noexcept {
        done.store(other.done.load());
        start_time = other.start_time;
        end_time = other.end_time;
        thread_id = other.thread_id;
        cpu_usage.store(other.cpu_usage.load());
        total_cpu_time = other.total_cpu_time;
    }

    ThreadStatus& operator=(const ThreadStatus& other) noexcept {
        if (this != &other) {
            done.store(other.done.load());
            start_time = other.start_time;
            end_time = other.end_time;
            thread_id = other.thread_id;
            cpu_usage.store(other.cpu_usage.load());
            total_cpu_time = other.total_cpu_time;
        }
        return *this;
    }

    ThreadStatus(ThreadStatus&& other) noexcept {
        done.store(other.done.load());
        start_time = other.start_time;
        end_time = other.end_time;
        thread_id = other.thread_id;
        cpu_usage.store(other.cpu_usage.load());
        total_cpu_time = other.total_cpu_time;
    }

    ThreadStatus& operator=(ThreadStatus&& other) noexcept {
        if (this != &other) {
            done.store(other.done.load());
            start_time = other.start_time;
            end_time = other.end_time;
            thread_id = other.thread_id;
            cpu_usage.store(other.cpu_usage.load());
            total_cpu_time = other.total_cpu_time;
        }
        return *this;
    }
};

std::vector<ThreadStatus> thread_statuses;
std::mutex cout_mutex;

void display_progress(int total_threads) {
    using namespace std::chrono;
    auto start = steady_clock::now();
    auto last_cpu_update = steady_clock::now();
    std::vector<ULONG64> prev_thread_times(total_threads, 0);
    ULONG64 prev_process_time = GetProcessCpuTime();

    while (true) {
        system("cls");
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Status\n\n";

        int completed = 0;
        for (int i = 0; i < total_threads; i++) {
            auto current_time = steady_clock::now();
            std::cout << "Thread " << i + 1 << ": ";
            if (thread_statuses[i].done) {
                auto duration = duration_cast<milliseconds>(thread_statuses[i].end_time - thread_statuses[i].start_time).count();
                std::cout << "Finished in " << duration << " ms";
                completed++;
            }
            else {
                if (thread_statuses[i].start_time.time_since_epoch().count() == 0) {
                    std::cout << "Not started";
                }
                else {
                    auto running_time = duration_cast<milliseconds>(current_time - thread_statuses[i].start_time).count();
                    std::cout << "Running (" << running_time << " ms)";
                }
            }

            std::cout << " [CPU: " << std::fixed << std::setprecision(1) << thread_statuses[i].cpu_usage.load() << "%]";
            std::cout << std::endl;
        }

        auto total_elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
        std::cout << completed << "/" << total_threads << std::endl;

        if (duration_cast<milliseconds>(steady_clock::now() - last_cpu_update).count() >= 1000) {
            last_cpu_update = steady_clock::now();
            ULONG64 current_process_time = GetProcessCpuTime();
            ULONG64 process_delta = current_process_time - prev_process_time;
            prev_process_time = current_process_time;
            double process_usage = process_delta / 10.0;
            double total_threads_usage = 0.0;

            for (int i = 0; i < total_threads; i++) {
                if (thread_statuses[i].thread_id != 0 && !thread_statuses[i].done) {
                    ULONG64 current_thread_time = GetThreadCpuTime(thread_statuses[i].thread_id);
                    ULONG64 thread_delta = current_thread_time - prev_thread_times[i];
                    prev_thread_times[i] = current_thread_time;
                    double thread_usage = thread_delta / 10.0;
                    thread_statuses[i].cpu_usage.store(thread_usage);
                    total_threads_usage += thread_usage;
                }
            }

            std::cout << "----------------------------------------" << std::endl;
            std::cout << "Process CPU usage: " << std::fixed << std::setprecision(1) << process_usage << "% (of one core)" << std::endl;
            std::cout << "Total threads CPU usage: " << std::fixed << std::setprecision(1) << total_threads_usage << "% (of one core)" << std::endl;

            if (process_usage > 0) {
                double efficiency = (total_threads_usage / process_usage) * 100.0;
                std::cout << "Threads efficiency: " << std::fixed << std::setprecision(1) << efficiency << "%" << std::endl;
            }
        }

        if (completed == total_threads) break;
        std::this_thread::sleep_for(milliseconds(1));
    }
}

std::string show_config() {
    std::string resp = "Amount of threads: " + std::to_string(config.threads) + "\n";
    resp += "Array size: " + std::to_string(config.arraySize) + "\n";
    resp += "Random value: ";
    resp += config.randomValue ? "on" : "off";
    resp += "\n";
    resp += "Show value: ";
    resp += config.showValue ? "on" : "off";
    resp += "\nType of sort: ";
    switch (config.type_of_sort) {
    case 0: resp += "qSort"; break;
    case 1: resp += "Buble sort"; break;
    }
    return resp;
}

void genV(std::vector<int>& v) {
    for (auto& i : v) i = static_cast<int>(gen());
}

void inputV(std::vector<int>& v) {
    for (auto& i : v) getInt(i);
}

void bubble_sort(std::vector<int>& v) {
    int n = (int)v.size();
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            if (v[j] > v[j + 1]) {
                std::swap(v[j], v[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

void quick_sort_helper(std::vector<int>& v, int low, int high) {
    if (low < high) {
        int pivot = v[high];
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (v[j] <= pivot) {
                i++;
                std::swap(v[i], v[j]);
            }
        }
        std::swap(v[i + 1], v[high]);
        int pi = i + 1;
        quick_sort_helper(v, low, pi - 1);
        quick_sort_helper(v, pi + 1, high);
    }
}

void quick_sort(std::vector<int>& v) {
    if (!v.empty()) quick_sort_helper(v, 0, (int)v.size() - 1);
}

std::vector<int> merge_sorted_chunks(const std::vector<std::vector<int>>& chunks) {
    if (chunks.empty()) return {};
    std::vector<int> result = chunks[0];

    for (size_t i = 1; i < chunks.size(); i++) {
        std::vector<int> merged;
        merged.reserve(result.size() + chunks[i].size());
        size_t idx1 = 0, idx2 = 0;

        while (idx1 < result.size() && idx2 < chunks[i].size()) {
            if (result[idx1] <= chunks[i][idx2]) {
                merged.push_back(result[idx1++]);
            }
            else {
                merged.push_back(chunks[i][idx2++]);
            }
        }

        while (idx1 < result.size()) merged.push_back(result[idx1++]);
        while (idx2 < chunks[i].size()) merged.push_back(chunks[i][idx2++]);
        result = std::move(merged);
    }

    return result;
}

struct ThreadData {
    std::vector<int>* chunk;
    ThreadStatus* status;
    int type_of_sort;
};

DWORD WINAPI SortThread(LPVOID lpParameter) {
    ThreadData* data = static_cast<ThreadData*>(lpParameter);
    data->status->start_time = std::chrono::steady_clock::now();
    data->status->thread_id = GetCurrentThreadId();

    if (data->type_of_sort == 0) {
        quick_sort(*data->chunk);
    }
    else {
        bubble_sort(*data->chunk);
    }

    data->status->end_time = std::chrono::steady_clock::now();
    data->status->total_cpu_time = GetThreadCpuTime(data->status->thread_id);
    data->status->done = true;
    return 0;
}

void sort_chunks(std::vector<int> v) {
    system("cls");
    int n = (int)v.size();
    int T = std::max(1, static_cast<int>(config.threads));
    std::vector<std::vector<int>> chunks(T);
    int base = n / T;
    int rem = n % T;
    int idx = 0;

    for (int i = 0; i < T; ++i) {
        int thisSize = base + (i < rem ? 1 : 0);
        for (int j = 0; j < thisSize; ++j) {
            chunks[i].push_back(v[idx++]);
        }
    }

    thread_statuses.clear();
    thread_statuses.resize(chunks.size());
    auto total_start_time = std::chrono::steady_clock::now();
    
    std::thread progress_thread(display_progress, (int)chunks.size());
    std::vector<ThreadData> thread_data(chunks.size());
    std::vector<HANDLE> thread_handles(chunks.size());
    
    for (int i = 0; i < (int)chunks.size(); i++) {
        thread_data[i].chunk = &chunks[i];
        thread_data[i].status = &thread_statuses[i];
        thread_data[i].type_of_sort = config.type_of_sort;
        
        thread_handles[i] = CreateThread(
            NULL,                   
            0,                      
            SortThread,             
            &thread_data[i],        
            0,                      
            NULL                    
        );
        
        if (thread_handles[i] == NULL) {
            std::cerr << "Error creating thread " << i << std::endl;
        }
    }

    WaitForMultipleObjects(thread_handles.size(), thread_handles.data(), TRUE, INFINITE);
    
    for (HANDLE handle : thread_handles) {
        CloseHandle(handle);
    }
    
    progress_thread.join();

    auto total_end_time = std::chrono::steady_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();
    std::vector<int> sorted_array = merge_sorted_chunks(chunks);
    bool is_sorted = true;

    for (size_t i = 1; i < sorted_array.size(); i++) {
        if (sorted_array[i] < sorted_array[i - 1]) {
            is_sorted = false;
            break;
        }
    }

    system("cls");
    std::cout << "=== SORTING COMPLETED ===\n\n";
    ULONG64 total_cpu_time = 0;

    for (int i = 0; i < (int)chunks.size(); i++) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(thread_statuses[i].end_time - thread_statuses[i].start_time).count();
        long double avg = (thread_statuses[i].total_cpu_time * 100.0 / duration);
        if (duration == 0) avg = 0;
        std::cout << "Thread " << i + 1 << ":\n";
        std::cout << "  Execution time: " << duration << " ms\n";
        std::cout << "  CPU time: " << thread_statuses[i].total_cpu_time << " ms\n";
        std::cout << "  CPU usage: " << std::fixed << std::setprecision(1) << avg << "%\n";
        std::cout << "  Elements sorted: " << chunks[i].size() << "\n\n";
        total_cpu_time += thread_statuses[i].total_cpu_time;
    }
    long double avg = (total_cpu_time * 100.0 / total_duration);
    if (total_duration == 0) avg = 0;
    std::cout << "=== SUMMARY ===\n";
    std::cout << "Total execution time: " << total_duration << " ms\n";
    std::cout << "Total CPU time: " << total_cpu_time << " ms\n";
    std::cout << "Average CPU usage: " << std::fixed << std::setprecision(1) << avg << "%\n";
    std::cout << "Array size: " << sorted_array.size() << "\n";
    std::cout << "Array is " << (is_sorted ? "sorted correctly" : "NOT sorted correctly") << "\n";

    if (config.showValue && !sorted_array.empty()) {
        std::cout << "\nSorted elements: ";
        for(auto& i:sorted_array)
            std::cout<<i<<" ";
        std::cout << "\n";
    }

    std::cout << "\n";
    system("pause");
}

void start() {
    system("cls");
    std::cout << "Fill data...\n";
    std::vector<int> v(config.arraySize);
    if (config.randomValue) genV(v); else inputV(v);

    system("cls");
    if (config.showValue) {
        if (!v.empty()) {
            std::cout << "Array = {" << v[0];
            for (size_t i = 1; i < v.size(); i++) std::cout << ", " << v[i];
            std::cout << "}\n";
        }
        else std::cout << "Array is empty\n";
    }
    system("pause");
    sort_chunks(v);
}

void edit_config() {
    system("cls");
    std::cout << "Treads count: " << config.threads << "\n";
    getInt(config.threads, 0, 1, 30, "New value of threads (Hardware max: " + std::to_string(std::thread::hardware_concurrency()) + ")");
    system("cls");
    std::cout << "Array size: " << config.arraySize << "\n";
    getInt(config.arraySize, 0, 1, 1000000, "New value of array size");
    system("cls");
    std::string s = config.randomValue ? "on" : "off";
    std::cout << "Randomizer status: " << s << "\n";
    int v;
    getInt(v, 0, 0, 1, "New value of randomizer (0 - off, 1 - on)");
    config.randomValue = v;
    system("cls");
    s = config.showValue ? "on" : "off";
    std::cout << "Show status: " << s << "\n";
    getInt(v, 0, 0, 1, "New value of show (0 - off, 1 - on)");
    config.showValue = v;
    system("cls");
    s = config.type_of_sort ? "Bubble sort" : "qSort";
    std::cout << "Type of sort algo: " << s << "\n";
    getInt(config.type_of_sort, 0, 0, 1, "New value of type of sort algo (0 - qSort, 1 - Bubble Sort)");
}

void testing()
{
    system("cls");
    std::cout<<"Regen values?\n1. Yes\n2. No\n0. Exit\n";
    int choose;
    getInt(choose, 0,0,2);
    switch (choose)
    {
    case 1:
        test.resize(config.arraySize);
        genV(test);
        break;
    case 2:
        break;
    case 0:
        return;
        break;
    }    
    sort_chunks(test);
}

void main_menu() {
    while (true) {
        system("cls");
        std::cout << "1. Start\n2. Edit settings\n3. Testing\n4. Exit\n\n" << show_config() << '\n';
        int choose;
        getInt(choose, 0, 1, 4, "Choose option");
        switch (choose) {
        case 1: start(); break;
        case 2: edit_config(); break;
        case 3: testing(); break;
        case 4: return;
        }
    }
}

int main() {
    main_menu();
    return 0;
}