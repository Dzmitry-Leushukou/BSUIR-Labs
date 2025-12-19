#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>
#include <string>
#include <iomanip>
#include <atomic>
#include <mutex>

class FileProcessor {
private:
    std::string filename_;
    size_t file_size_;
    int num_threads_;
    std::mutex file_mutex_;

public:
    FileProcessor(const std::string& filename, size_t file_size, int num_threads = 1)
        : filename_(filename), file_size_(file_size), num_threads_(num_threads) {
    }

    double traditional_single_thread() {
        auto start = std::chrono::high_resolution_clock::now();

        HANDLE hFile = CreateFileA(filename_.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            std::cerr << "Cannot open file for reading" << std::endl;
            return -1;
        }

        std::vector<char> buffer(file_size_);
        DWORD bytesRead;
        if (!ReadFile(hFile, buffer.data(), (DWORD)file_size_, &bytesRead, NULL)) {
            std::cerr << "Read failed" << std::endl;
            CloseHandle(hFile);
            return -1;
        }
        CloseHandle(hFile);

        process_data(buffer.data(), 0, buffer.size());

        hFile = CreateFileA(filename_.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            std::cerr << "Cannot open file for writing" << std::endl;
            return -1;
        }

        DWORD bytesWritten;
        if (!WriteFile(hFile, buffer.data(), (DWORD)file_size_, &bytesWritten, NULL)) {
            std::cerr << "Write failed" << std::endl;
            CloseHandle(hFile);
            return -1;
        }
        CloseHandle(hFile);

        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }

    double traditional_multi_thread() {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<char> buffer;
        {
            std::lock_guard<std::mutex> lock(file_mutex_);
            HANDLE hFile = CreateFileA(filename_.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                std::cerr << "Cannot open file for reading" << std::endl;
                return -1;
            }

            buffer.resize(file_size_);
            DWORD bytesRead;
            if (!ReadFile(hFile, buffer.data(), (DWORD)file_size_, &bytesRead, NULL)) {
                std::cerr << "Read failed" << std::endl;
                CloseHandle(hFile);
                return -1;
            }
            CloseHandle(hFile);
        }

        std::vector<std::thread> threads;
        std::atomic<size_t> next_chunk{ 0 };
        const size_t chunk_size = 64 * 1024; 

        for (int i = 0; i < num_threads_; ++i) {
            threads.emplace_back([this, &buffer, &next_chunk, chunk_size]() {
                process_data_chunked(buffer.data(), file_size_, next_chunk, chunk_size);
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        {
            std::lock_guard<std::mutex> lock(file_mutex_);
            HANDLE hFile = CreateFileA(filename_.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                std::cerr << "Cannot open file for writing" << std::endl;
                return -1;
            }

            DWORD bytesWritten;
            if (!WriteFile(hFile, buffer.data(), (DWORD)file_size_, &bytesWritten, NULL)) {
                std::cerr << "Write failed" << std::endl;
                CloseHandle(hFile);
                return -1;
            }
            CloseHandle(hFile);
        }

        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }

    double memory_mapped_single_thread() {
        auto start = std::chrono::high_resolution_clock::now();

        std::lock_guard<std::mutex> lock(file_mutex_);

        HANDLE hFile = CreateFileA(filename_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            std::cerr << "Cannot open file" << std::endl;
            return -1;
        }

        HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, (DWORD)file_size_, NULL);
        if (hMapping == NULL) {
            std::cerr << "Cannot create file mapping" << std::endl;
            CloseHandle(hFile);
            return -1;
        }

        char* mapped_data = static_cast<char*>(MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, (SIZE_T)file_size_));
        if (mapped_data == NULL) {
            std::cerr << "Cannot map view of file" << std::endl;
            CloseHandle(hMapping);
            CloseHandle(hFile);
            return -1;
        }

        process_data(mapped_data, 0, file_size_);

        UnmapViewOfFile(mapped_data);
        CloseHandle(hMapping);
        CloseHandle(hFile);

        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }

    double memory_mapped_multi_thread() {
        auto start = std::chrono::high_resolution_clock::now();

        HANDLE hFile = INVALID_HANDLE_VALUE;
        HANDLE hMapping = NULL;
        char* mapped_data = nullptr;

        {
            std::lock_guard<std::mutex> lock(file_mutex_);
            hFile = CreateFileA(filename_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                std::cerr << "Cannot open file" << std::endl;
                return -1;
            }

            hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, (DWORD)file_size_, NULL);
            if (hMapping == NULL) {
                std::cerr << "Cannot create file mapping" << std::endl;
                CloseHandle(hFile);
                return -1;
            }

            mapped_data = static_cast<char*>(MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, (SIZE_T)file_size_));
            if (mapped_data == NULL) {
                std::cerr << "Cannot map view of file" << std::endl;
                CloseHandle(hMapping);
                CloseHandle(hFile);
                return -1;
            }
        }

        std::vector<std::thread> threads;
        std::atomic<size_t> next_chunk{ 0 };
        const size_t chunk_size = 64 * 1024; 

        for (int i = 0; i < num_threads_; ++i) {
            threads.emplace_back([this, mapped_data, &next_chunk, chunk_size]() {
                process_data_chunked(mapped_data, file_size_, next_chunk, chunk_size);
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        {
            std::lock_guard<std::mutex> lock(file_mutex_);
            UnmapViewOfFile(mapped_data);
            CloseHandle(hMapping);
            CloseHandle(hFile);
        }

        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }

private:
    void process_data(char* data, size_t start, size_t end) {
        const char key = 0xAA;
        for (size_t i = start; i < end; ++i) {
            data[i] ^= key;
        }
    }

    void process_data_chunked(char* data, size_t total_size, std::atomic<size_t>& next_chunk, size_t chunk_size) {
        const char key = 0xAA;

        while (true) {
            size_t chunk_start = next_chunk.fetch_add(chunk_size);

            if (chunk_start >= total_size) {
                break; 
            }

            size_t chunk_end = (chunk_start + chunk_size < total_size) ? chunk_start + chunk_size : total_size;

            for (size_t i = chunk_start; i < chunk_end; ++i) {
                data[i] ^= key;
            }
        }
    }
};

bool create_test_file_fast(const std::string& filename, size_t size) {
    std::mutex file_mutex;
    std::lock_guard<std::mutex> lock(file_mutex);

    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Cannot create test file" << std::endl;
        return false;
    }

    const size_t BUFFER_SIZE = 64 * 1024;
    std::vector<char> buffer(BUFFER_SIZE);

    std::minstd_rand fast_gen(42);
    std::uniform_int_distribution<> dis(0, 255);

    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = static_cast<char>(dis(fast_gen));
    }

    size_t remaining = size;
    while (remaining > 0) {
        size_t chunk_size = (BUFFER_SIZE < remaining) ? BUFFER_SIZE : remaining;
        DWORD bytesWritten;

        if (!WriteFile(hFile, buffer.data(), (DWORD)chunk_size, &bytesWritten, NULL)) {
            std::cerr << "Write failed" << std::endl;
            CloseHandle(hFile);
            return false;
        }

        remaining -= chunk_size;
    }

    CloseHandle(hFile);
    return true;
}

bool verify_data_integrity(const std::string& filename, size_t expected_size) {
    std::mutex file_mutex;
    std::lock_guard<std::mutex> lock(file_mutex);

    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    std::vector<char> buffer(expected_size);
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer.data(), (DWORD)expected_size, &bytesRead, NULL)) {
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    return bytesRead == expected_size;
}

int main() {
    const std::string test_filename = "test_data.bin";
    const std::string original_filename = "original_test_data.bin";
    const size_t file_size = 512 * 1024 * 1024; // 512 MB
    const std::vector<int> thread_counts = { 1, 2, 4, 8 };

    std::cout << "File Processing Performance Comparison\n";
    std::cout << "File size: " << file_size / (1024 * 1024) << " MB\n\n";

    std::cout << "Creating original test file... ";
    if (!create_test_file_fast(original_filename, file_size)) {
        std::cerr << "Failed to create test file!" << std::endl;
        return 1;
    }
    std::cout << "Done!\n\n";

    std::cout << std::left << std::setw(25) << "Method"
        << std::setw(10) << "Threads"
        << std::setw(15) << "Time (s)"
        << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(65, '-') << std::endl;

    double base_time = 0;

    for (int threads : thread_counts) {
        {
            std::mutex copy_mutex;
            std::lock_guard<std::mutex> lock(copy_mutex);
            CopyFileA(original_filename.c_str(), test_filename.c_str(), FALSE);
        }

        FileProcessor processor1(test_filename, file_size, threads);
        double time = processor1.traditional_multi_thread();

        if (threads == 1) {
            base_time = time;
            std::cout << std::left << std::setw(25) << "Traditional"
                << std::setw(10) << threads
                << std::setw(15) << std::fixed << std::setprecision(3) << time
                << std::setw(15) << "1.00x" << std::endl;
        }
        else {
            std::cout << std::left << std::setw(25) << "Traditional"
                << std::setw(10) << threads
                << std::setw(15) << std::fixed << std::setprecision(3) << time
                << std::setw(15) << std::fixed << std::setprecision(2) << (base_time / time) << "x" << std::endl;
        }

        {
            std::mutex copy_mutex;
            std::lock_guard<std::mutex> lock(copy_mutex);
            CopyFileA(original_filename.c_str(), test_filename.c_str(), FALSE);
        }

        FileProcessor processor2(test_filename, file_size, threads);
        time = processor2.memory_mapped_multi_thread();

        std::cout << std::left << std::setw(25) << "Memory Mapped"
            << std::setw(10) << threads
            << std::setw(15) << std::fixed << std::setprecision(3) << time
            << std::setw(15) << std::fixed << std::setprecision(2) << (base_time / time) << "x" << std::endl;

        std::cout << std::endl;
    }

    std::cout << "Verifying data integrity... ";
    if (verify_data_integrity(test_filename, file_size)) {
        std::cout << "OK" << std::endl;
    }
    else {
        std::cout << "FAILED" << std::endl;
    }

    {
        std::mutex delete_mutex;
        std::lock_guard<std::mutex> lock(delete_mutex);
        DeleteFileA(test_filename.c_str());
        DeleteFileA(original_filename.c_str());
    }

    return 0;
}