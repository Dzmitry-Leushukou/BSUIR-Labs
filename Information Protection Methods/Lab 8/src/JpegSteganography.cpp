#include "JpegSteganography.hpp"

#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace {
FILE* openFile(const std::filesystem::path& path, bool writing) {
    return _wfopen(path.c_str(), writing ? L"wb" : L"rb");
}
}

void JpegSteganography::jpegError(j_common_ptr codec) {
    auto* error = reinterpret_cast<Error*>(codec->err);
    codec->err->format_message(codec, error->message);
    std::longjmp(error->jump, 1);
}

JpegSteganography::JpegSteganography(const std::filesystem::path& image)
    : source_(image) {
    reader_.err = jpeg_std_error(&error_.manager);
    error_.manager.error_exit = jpegError;
    error_.manager.emit_message = [](j_common_ptr codec, int level) {
        if (level < 0) jpegError(codec);
    };
    if (setjmp(error_.jump)) {
        close();
        throw std::runtime_error(std::string("JPEG: ") + error_.message);
    }
    input_ = openFile(image, false);
    if (!input_) throw std::runtime_error("Cannot open input image.");
    jpeg_create_decompress(&reader_);
    jpeg_stdio_src(&reader_, input_);
    for (int marker = 0; marker < 16; ++marker)
        jpeg_save_markers(&reader_, JPEG_APP0 + marker, 0xffff);
    jpeg_save_markers(&reader_, JPEG_COM, 0xffff);
    jpeg_read_header(&reader_, TRUE);
    blocks_ = jpeg_read_coefficients(&reader_);
    if (!blocks_) {
        close();
        throw std::runtime_error("JPEG does not contain DCT coefficients.");
    }
}

JpegSteganography::~JpegSteganography() { close(); }

void JpegSteganography::close() noexcept {
    if (writer_.mem) jpeg_destroy_compress(&writer_);
    if (reader_.mem) jpeg_destroy_decompress(&reader_);
    if (output_) { std::fclose(output_); output_ = nullptr; }
    if (input_) { std::fclose(input_); input_ = nullptr; }
}

void JpegSteganography::visit(const std::function<bool(JCOEF&)>& action, bool writing) {
    if (setjmp(error_.jump))
        throw std::runtime_error(std::string("JPEG: ") + error_.message);
    for (int c = 0; c < reader_.num_components; ++c) {
        const auto& component = reader_.comp_info[c];
        for (JDIMENSION y = 0; y < component.height_in_blocks; ++y) {
            auto row = reader_.mem->access_virt_barray(
                reinterpret_cast<j_common_ptr>(&reader_), blocks_[c], y, 1, writing);
            for (JDIMENSION x = 0; x < component.width_in_blocks; ++x)
                for (int k = 1; k < DCTSIZE2; ++k) {
                    auto& coefficient = row[0][x][k];
                    if (std::abs(int(coefficient)) >= 2 && !action(coefficient)) return;
                }
        }
    }
}

std::size_t JpegSteganography::availableBits() {
    std::size_t count = 0;
    visit([&](JCOEF&) { ++count; return true; });
    return count;
}

std::size_t JpegSteganography::capacity() {
    const auto bytes = availableBits() / 8;
    if (bytes < fixedHeaderSize + 1) return 0;
    auto length = bytes - fixedHeaderSize - 1;
    while (length > bytes - fixedHeaderSize - lengthSize(length)) --length;
    return length;
}

std::size_t JpegSteganography::lengthSize(std::size_t length) {
    std::size_t size = 1;
    while (length >>= 7) ++size;
    return size;
}

void JpegSteganography::putLength(std::vector<unsigned char>& data, std::size_t length) {
    do {
        const auto byte = static_cast<unsigned char>(length & 0x7f);
        length >>= 7;
        data.push_back(byte | (length ? 0x80 : 0));
    } while (length);
}

std::size_t JpegSteganography::getLength(const unsigned char* data, std::size_t count) {
    std::size_t length = 0;
    for (std::size_t i = 0; i < count; ++i) {
        const auto value = std::size_t(data[i] & 0x7f);
        const auto shift = i * 7;
        if (shift >= std::numeric_limits<std::size_t>::digits ||
            value > (std::numeric_limits<std::size_t>::max() >> shift))
            throw std::runtime_error("Invalid message length.");
        length |= value << shift;
    }
    if (count > 1 && (data[count - 1] & 0x7f) == 0)
        throw std::runtime_error("Non-canonical message length.");
    return length;
}

std::uint32_t JpegSteganography::checksum(const std::string& text) {
    std::uint32_t crc = 0xffffffffu;
    for (unsigned char byte : text) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320u : 0u);
    }
    return crc ^ 0xffffffffu;
}

void JpegSteganography::put32(std::vector<unsigned char>& data, std::uint32_t value) {
    for (int shift = 24; shift >= 0; shift -= 8)
        data.push_back(static_cast<unsigned char>(value >> shift));
}

std::uint32_t JpegSteganography::get32(const unsigned char* data) {
    std::uint32_t value = 0;
    for (int i = 0; i < 4; ++i) value = (value << 8) | data[i];
    return value;
}

void JpegSteganography::hide(const std::string& text, const std::filesystem::path& output) {
    const auto bytes = availableBits() / 8;
    const auto headerSize = fixedHeaderSize + lengthSize(text.size());
    if (bytes < headerSize || text.size() > bytes - headerSize)
        throw std::runtime_error("Message is too large for this JPEG.");

    std::vector<unsigned char> data{'S', 'T', 'G', '9'};
    putLength(data, text.size());
    put32(data, checksum(text));
    data.insert(data.end(), text.begin(), text.end());

    std::size_t bit = 0;
    visit([&](JCOEF& coefficient) {
        const int value = (data[bit / 8] >> (7 - bit % 8)) & 1;
        const int magnitude = (std::abs(int(coefficient)) & ~1) | value;
        coefficient = static_cast<JCOEF>(coefficient < 0 ? -magnitude : magnitude);
        return ++bit < data.size() * 8;
    }, true);
    save(output);
}

std::string JpegSteganography::extract() {
    std::vector<unsigned char> data;
    constexpr auto maxLengthBytes = (std::numeric_limits<std::size_t>::digits + 6) / 7;
    std::size_t lengthEnd = 0;
    std::size_t bit = 0;
    visit([&](JCOEF& coefficient) {
        if (bit % 8 == 0) data.push_back(0);
        data[bit / 8] |= (std::abs(int(coefficient)) & 1) << (7 - bit % 8);
        if (++bit % 8 != 0) return true;
        if (data.size() == 4 && std::memcmp(data.data(), "STG9", 4) != 0)
            throw std::runtime_error("No Lab8 message found in this JPEG.");
        if (data.size() > 4 && !lengthEnd) {
            if ((data.back() & 0x80) == 0) lengthEnd = data.size();
            else if (data.size() == 4 + maxLengthBytes)
                throw std::runtime_error("Invalid message length.");
        }
        return !lengthEnd || data.size() < lengthEnd + 4;
    });
    if (!lengthEnd || bit != (lengthEnd + 4) * 8)
        throw std::runtime_error("No Lab8 message found in this JPEG.");
    const auto headerSize = lengthEnd + 4;
    const auto length = getLength(data.data() + 4, lengthEnd - 4);
    const auto expectedCrc = get32(data.data() + lengthEnd);
    const auto bytes = availableBits() / 8;
    if (headerSize > bytes || length > bytes - headerSize)
        throw std::runtime_error("Invalid message length.");

    data.assign(headerSize + std::size_t(length), 0);
    bit = 0;
    visit([&](JCOEF& coefficient) {
        data[bit / 8] |= (std::abs(int(coefficient)) & 1) << (7 - bit % 8);
        return ++bit < data.size() * 8;
    });
    std::string text(data.begin() + headerSize, data.end());
    if (checksum(text) != expectedCrc)
        throw std::runtime_error("Message checksum mismatch: data is damaged.");
    return text;
}

void JpegSteganography::save(const std::filesystem::path& output) {
    if (std::filesystem::exists(output) && std::filesystem::equivalent(source_, output))
        throw std::runtime_error("Input and output image must be different files.");
    writer_.err = &error_.manager;
    if (setjmp(error_.jump))
        throw std::runtime_error(std::string("JPEG: ") + error_.message);
    output_ = openFile(output, true);
    if (!output_) throw std::runtime_error("Cannot open output image.");
    jpeg_create_compress(&writer_);
    jpeg_stdio_dest(&writer_, output_);
    jpeg_copy_critical_parameters(&reader_, &writer_);
    jpeg_write_coefficients(&writer_, blocks_);
    for (auto marker = reader_.marker_list; marker; marker = marker->next) {
        const bool jfif = marker->marker == JPEG_APP0 && marker->data_length >= 5 &&
                          std::memcmp(marker->data, "JFIF\0", 5) == 0;
        const bool adobe = marker->marker == JPEG_APP0 + 14 && marker->data_length >= 5 &&
                           std::memcmp(marker->data, "Adobe", 5) == 0;
        if ((jfif && writer_.write_JFIF_header) || (adobe && writer_.write_Adobe_marker)) continue;
        jpeg_write_marker(&writer_, marker->marker, marker->data, marker->data_length);
    }
    jpeg_finish_compress(&writer_);
    jpeg_destroy_compress(&writer_);
    const int result = std::fclose(output_);
    output_ = nullptr;
    if (result != 0) throw std::runtime_error("Cannot finish writing output image.");
}
