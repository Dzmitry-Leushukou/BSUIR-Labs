#pragma once

#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>
#include <jpeglib.h>

class JpegSteganography {
public:
    explicit JpegSteganography(const std::filesystem::path& image);
    ~JpegSteganography();
    JpegSteganography(const JpegSteganography&) = delete;
    JpegSteganography& operator=(const JpegSteganography&) = delete;

    std::size_t capacity();
    std::size_t availableBits();
    void hide(const std::string& text, const std::filesystem::path& output);
    std::string extract();

private:
    struct Error {
        jpeg_error_mgr manager{};
        std::jmp_buf jump;
        char message[JMSG_LENGTH_MAX]{};
    } error_;

    static constexpr std::size_t fixedHeaderSize = 8;
    jpeg_decompress_struct reader_{};
    jpeg_compress_struct writer_{};
    jvirt_barray_ptr* blocks_ = nullptr;
    FILE* input_ = nullptr;
    FILE* output_ = nullptr;
    std::filesystem::path source_;

    static void jpegError(j_common_ptr codec);
    static std::uint32_t checksum(const std::string& text);
    static void put32(std::vector<unsigned char>& data, std::uint32_t value);
    static std::uint32_t get32(const unsigned char* data);
    static std::size_t lengthSize(std::size_t length);
    static void putLength(std::vector<unsigned char>& data, std::size_t length);
    static std::size_t getLength(const unsigned char* data, std::size_t count);
    void close() noexcept;
    void visit(const std::function<bool(JCOEF&)>& action, bool writing = false);
    void save(const std::filesystem::path& output);
};
