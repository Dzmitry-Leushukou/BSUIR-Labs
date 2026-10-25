#include "crypto.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

using gost::Byte;
using gost::Bytes;
using gost::Point;
using gost::UInt256;

constexpr std::size_t kMessageBytes = 30;
constexpr std::array<char, 8> kMagic = {'E', 'C', 'E', 'L', 'G', '1', 0, 0};

const UInt256& curveB()
{
    static const UInt256 value = [] {
        UInt256 result;
        gost::fromHex("295f9bae7428ed9ccc20e7c359a9d41a22fccd9108e17bf7ba9337a6f8ae9513", result);
        return result;
    }();
    return value;
}

const UInt256& squareRootExponent()
{
    static const UInt256 value = [] {
        UInt256 result;
        gost::fromHex("3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff66", result);
        return result;
    }();
    return value;
}

UInt256 shiftLeft8(const UInt256& value)
{
    UInt256 result;
    for (unsigned i = 0; i < 4; ++i) {
        result.limb[i] = value.limb[i] << 8;
        if (i != 0) result.limb[i] |= value.limb[i - 1] >> 56;
    }
    return result;
}

UInt256 shiftRight8(const UInt256& value)
{
    UInt256 result;
    for (unsigned i = 0; i < 4; ++i) {
        result.limb[i] = value.limb[i] >> 8;
        if (i != 3) result.limb[i] |= value.limb[i + 1] << 56;
    }
    return result;
}

UInt256 bytesToInteger(const Byte* data, std::size_t size)
{
    UInt256 result;
    for (std::size_t i = 0; i < size; ++i) {
        result = shiftLeft8(result);
        result.limb[0] |= data[i];
    }
    return result;
}

std::array<Byte, 32> integerToBytes(const UInt256& value)
{
    std::array<Byte, 32> result{};
    for (unsigned i = 0; i < result.size(); ++i) {
        result[i] = static_cast<Byte>(
            value.limb[(31 - i) / 8] >> (((31 - i) % 8) * 8));
    }
    return result;
}

bool readBytes(std::istream& input, Byte* data, std::size_t size)
{
    input.read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(size));
    return input.good() || input.gcount() == static_cast<std::streamsize>(size);
}

bool writeBytes(std::ostream& output, const Byte* data, std::size_t size)
{
    output.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    return output.good();
}

bool readPoint(std::istream& input, Point& point)
{
    std::array<Byte, 32> xBytes{};
    std::array<Byte, 32> yBytes{};
    if (!readBytes(input, xBytes.data(), xBytes.size()) ||
        !readBytes(input, yBytes.data(), yBytes.size())) {
        return false;
    }

    point.x = gost::fromBytes(xBytes);
    point.y = gost::fromBytes(yBytes);
    point.z = {};
    point.z.limb[0] = 1;
    point.infinity = false;
    return gost::isOnCurve(point.x, point.y);
}

bool writePoint(std::ostream& output, const Point& point)
{
    UInt256 x;
    UInt256 y;
    if (!gost::toAffine(point, x, y)) return false;
    const auto xBytes = integerToBytes(x);
    const auto yBytes = integerToBytes(y);
    return writeBytes(output, xBytes.data(), xBytes.size()) &&
           writeBytes(output, yBytes.data(), yBytes.size());
}

Point negatePoint(Point point)
{
    if (!point.infinity && !point.y.zero()) point.y = gost::subRaw(gost::field(), point.y);
    return point;
}

bool messagePoint(const Byte* data, std::size_t size, Point& point)
{
    const UInt256 message = bytesToInteger(data, size);
    UInt256 x = shiftLeft8(message);

    for (unsigned tag = 0; tag < 256; ++tag) {
        x.limb[0] = (x.limb[0] & ~static_cast<std::uint64_t>(0xff)) | tag;
        if (gost::compare(x, gost::field()) >= 0) continue;

        const UInt256 x2 = gost::mulMod(x, x, gost::field());
        const UInt256 x3 = gost::mulMod(x2, x, gost::field());
        const UInt256 rhs = gost::addMod(
            gost::addMod(x3, gost::mulMod(gost::curveA(), x, gost::field()), gost::field()),
            curveB(), gost::field());
        const UInt256 y = gost::powMod(rhs, squareRootExponent(), gost::field());
        if (gost::compare(gost::mulMod(y, y, gost::field()), rhs) == 0) {
            point = gost::affinePoint(x, y);
            return true;
        }
    }
    return false;
}

bool pointMessage(const Point& point, std::array<Byte, kMessageBytes>& result)
{
    UInt256 x;
    UInt256 y;
    if (!gost::toAffine(point, x, y)) return false;
    const UInt256 message = shiftRight8(x);

    // The encoding reserves the upper 16 bits of the field element.
    if ((message.limb[3] >> 48) != 0) return false;

    const auto bytes = integerToBytes(message);
    std::copy(bytes.end() - static_cast<std::ptrdiff_t>(result.size()), bytes.end(), result.begin());
    return true;
}

bool readFile(const std::string& path, Bytes& data)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) return false;
    data.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return input.eof() || !input.fail();
}

bool writeUInt64(std::ostream& output, std::uint64_t value)
{
    std::array<Byte, 8> bytes{};
    for (unsigned i = 0; i < bytes.size(); ++i) bytes[i] = static_cast<Byte>(value >> (i * 8));
    return writeBytes(output, bytes.data(), bytes.size());
}

bool readUInt64(std::istream& input, std::uint64_t& value)
{
    std::array<Byte, 8> bytes{};
    if (!readBytes(input, bytes.data(), bytes.size())) return false;
    value = 0;
    for (unsigned i = 0; i < bytes.size(); ++i) value |= static_cast<std::uint64_t>(bytes[i]) << (i * 8);
    return true;
}

bool validPublicKey(const UInt256& x, const UInt256& y)
{
    if (!gost::isOnCurve(x, y)) return false;
    const Point key = gost::affinePoint(x, y);
    return gost::scalarMultiply(key, gost::order()).infinity;
}

bool generateKeys(const std::string& privatePath, const std::string& publicPath)
{
    if (privatePath == publicPath) return false;

    UInt256 privateKey;
    UInt256 publicX;
    UInt256 publicY;
    if (!gost::randomScalar(privateKey) ||
        !gost::toAffine(gost::scalarMultiply(
                            gost::affinePoint(gost::generatorX(), gost::generatorY()), privateKey),
                        publicX, publicY)) {
        return false;
    }

    return gost::writeFields(privatePath, {{"d", privateKey}}) &&
           gost::writeFields(publicPath, {{"x", publicX}, {"y", publicY}});
}

bool encryptFile(const std::string& publicPath,
                 const std::string& inputPath,
                 const std::string& outputPath)
{
    if (outputPath == publicPath || outputPath == inputPath) return false;

    std::vector<UInt256> publicFields;
    Bytes input;
    if (!gost::parseFields(publicPath, {"x", "y"}, publicFields) ||
        !validPublicKey(publicFields[0], publicFields[1]) ||
        !readFile(inputPath, input)) {
        return false;
    }

    const Point publicKey = gost::affinePoint(publicFields[0], publicFields[1]);
    const Point generator = gost::affinePoint(gost::generatorX(), gost::generatorY());
    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) return false;

    if (!writeBytes(output, reinterpret_cast<const Byte*>(kMagic.data()), kMagic.size()) ||
        !writeBytes(output, reinterpret_cast<const Byte*>("\x01\x1e\x00\x00"), 4) ||
        !writeUInt64(output, static_cast<std::uint64_t>(input.size()))) {
        return false;
    }

    for (std::size_t offset = 0; offset < input.size(); offset += kMessageBytes) {
        const std::size_t size = std::min(kMessageBytes, input.size() - offset);
        Point message;
        if (!messagePoint(input.data() + offset, size, message)) return false;

        UInt256 nonce;
        if (!gost::randomScalar(nonce)) return false;
        const Point c1 = gost::scalarMultiply(generator, nonce);
        const Point shared = gost::scalarMultiply(publicKey, nonce);
        const Point c2 = gost::pointAdd(message, shared);
        if (!writePoint(output, c1) || !writePoint(output, c2)) return false;
    }

    return output.good();
}

bool decryptFile(const std::string& privatePath,
                 const std::string& inputPath,
                 const std::string& outputPath)
{
    if (outputPath == privatePath || outputPath == inputPath) return false;

    std::vector<UInt256> privateFields;
    if (!gost::parseFields(privatePath, {"d"}, privateFields) ||
        privateFields[0].zero() || gost::compare(privateFields[0], gost::order()) >= 0) {
        return false;
    }

    std::ifstream input(inputPath, std::ios::binary);
    if (!input) return false;

    std::array<Byte, 8> magic{};
    if (!readBytes(input, magic.data(), magic.size()) || magic !=
        std::array<Byte, 8>{'E', 'C', 'E', 'L', 'G', '1', 0, 0}) {
        return false;
    }

    std::array<Byte, 4> header{};
    if (!readBytes(input, header.data(), header.size()) ||
        header[0] != 1 || header[1] != kMessageBytes || header[2] != 0 || header[3] != 0) {
        return false;
    }

    std::uint64_t originalSize = 0;
    if (!readUInt64(input, originalSize) ||
        originalSize > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        return false;
    }

    const std::size_t size = static_cast<std::size_t>(originalSize);
    const std::size_t blockCount = size == 0 ? 0 : (size + kMessageBytes - 1) / kMessageBytes;
    Bytes output;
    output.reserve(size);

    for (std::size_t block = 0; block < blockCount; ++block) {
        Point c1;
        Point c2;
        if (!readPoint(input, c1) || !readPoint(input, c2)) return false;

        const Point shared = gost::scalarMultiply(c1, privateFields[0]);
        const Point message = gost::pointAdd(c2, negatePoint(shared));
        std::array<Byte, kMessageBytes> decoded{};
        if (!pointMessage(message, decoded)) return false;

        const std::size_t blockSize = std::min(kMessageBytes, size - block * kMessageBytes);
        output.insert(output.end(), decoded.end() - static_cast<std::ptrdiff_t>(blockSize), decoded.end());
    }

    Byte trailing = 0;
    if (input.read(reinterpret_cast<char*>(&trailing), 1)) return false;
    if (!input.eof() || output.size() != size) return false;

    std::ofstream result(outputPath, std::ios::binary | std::ios::trunc);
    if (!result) return false;
    return writeBytes(result, output.data(), output.size()) && result.good();
}

void printUsage(const char* program)
{
    std::cerr << "Usage:\n"
              << "  " << program << " keygen <private_key_file> <public_key_file>\n"
              << "  " << program << " encrypt <public_key_file> <input_file> <encrypted_file>\n"
              << "  " << program << " decrypt <private_key_file> <encrypted_file> <output_file>\n";
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc == 4 && std::string(argv[1]) == "keygen") {
        if (!generateKeys(argv[2], argv[3])) {
            std::cerr << "Error: key generation failed\n";
            return 1;
        }
        std::cout << "Key pair generated\n";
        return 0;
    }

    if (argc == 5 && std::string(argv[1]) == "encrypt") {
        if (!encryptFile(argv[2], argv[3], argv[4])) {
            std::cerr << "Error: encryption failed\n";
            return 1;
        }
        std::cout << "File encrypted\n";
        return 0;
    }

    if (argc == 5 && std::string(argv[1]) == "decrypt") {
        if (!decryptFile(argv[2], argv[3], argv[4])) {
            std::cerr << "Error: decryption failed\n";
            return 1;
        }
        std::cout << "File decrypted\n";
        return 0;
    }

    printUsage(argv[0]);
    return 1;
}
