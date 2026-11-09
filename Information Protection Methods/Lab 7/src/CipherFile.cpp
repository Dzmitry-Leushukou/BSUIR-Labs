#include "CipherFile.hpp"
#include "ElGamal.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>

bool CipherFile::readBytes(std::istream& input, Byte* data, std::size_t size)
{
    input.read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(size));
    return input.good() || input.gcount() == static_cast<std::streamsize>(size);
}

bool CipherFile::writeBytes(std::ostream& output, const Byte* data, std::size_t size)
{
    output.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    return output.good();
}

bool CipherFile::readPoint(std::istream& input, CurvePoint& point)
{
    std::array<Byte, 32> xBytes{};
    std::array<Byte, 32> yBytes{};
    if (!readBytes(input, xBytes.data(), xBytes.size()) ||
        !readBytes(input, yBytes.data(), yBytes.size())) {
        return false;
    }

    point.x = UInt256::fromBytes(xBytes);
    point.y = UInt256::fromBytes(yBytes);
    point.z = {};
    point.z.limb[0] = 1;
    point.infinity = false;
    return EllipticCurve::isOnCurve(point.x, point.y);
}

bool CipherFile::writePoint(std::ostream& output, const CurvePoint& point)
{
    UInt256 x;
    UInt256 y;
    if (!EllipticCurve::toAffine(point, x, y)) return false;
    const auto xBytes = x.toBytes();
    const auto yBytes = y.toBytes();
    return writeBytes(output, xBytes.data(), xBytes.size()) &&
           writeBytes(output, yBytes.data(), yBytes.size());
}

bool CipherFile::readFile(const std::string& path, Bytes& data)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) return false;
    data.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return input.eof() || !input.fail();
}

bool CipherFile::writeUInt64(std::ostream& output, std::uint64_t value)
{
    std::array<Byte, 8> bytes{};
    for (unsigned i = 0; i < bytes.size(); ++i) bytes[i] = static_cast<Byte>(value >> (i * 8));
    return writeBytes(output, bytes.data(), bytes.size());
}

bool CipherFile::readUInt64(std::istream& input, std::uint64_t& value)
{
    std::array<Byte, 8> bytes{};
    if (!readBytes(input, bytes.data(), bytes.size())) return false;
    value = 0;
    for (unsigned i = 0; i < bytes.size(); ++i) value |= static_cast<std::uint64_t>(bytes[i]) << (i * 8);
    return true;
}

bool CipherFile::encryptFile(const std::string& publicPath,
                             const std::string& inputPath,
                             const std::string& outputPath)
{
    if (outputPath == publicPath || outputPath == inputPath) return false;

    CurvePoint publicKey;
    Bytes input;
    if (!CipherFile::readPublicKey(publicPath, publicKey) ||
        !readFile(inputPath, input)) {
        return false;
    }

    const ElGamal cipher(publicKey);
    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) return false;

    if (!writeBytes(output, reinterpret_cast<const Byte*>(Magic.data()), Magic.size()) ||
        !writeBytes(output, reinterpret_cast<const Byte*>("\x01\x1e\x00\x00"), 4) ||
        !writeUInt64(output, static_cast<std::uint64_t>(input.size()))) {
        return false;
    }

    for (std::size_t offset = 0; offset < input.size(); offset += ElGamal::BlockSize) {
        const std::size_t size = std::min(ElGamal::BlockSize, input.size() - offset);
        CurvePoint message;
        if (!ElGamal::messagePoint(input.data() + offset, size, message)) return false;

        CipherBlock block;
        if (!cipher.encrypt(message, block) ||
            !writePoint(output, block.c1) || !writePoint(output, block.c2)) return false;
    }

    output.flush();
    return output.good();
}

bool CipherFile::decryptFile(const std::string& privatePath,
                             const std::string& inputPath,
                             const std::string& outputPath)
{
    if (outputPath == privatePath || outputPath == inputPath) return false;

    UInt256 privateKey;
    if (!CipherFile::readPrivateKey(privatePath, privateKey)) {
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
        header[0] != 1 || header[1] != ElGamal::BlockSize || header[2] != 0 || header[3] != 0) {
        return false;
    }

    std::uint64_t originalSize = 0;
    if (!readUInt64(input, originalSize) ||
        originalSize > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
        return false;
    }

    const std::size_t size = static_cast<std::size_t>(originalSize);
    const std::size_t blockCount = size / ElGamal::BlockSize +
                                 (size % ElGamal::BlockSize != 0);
    const auto payloadStart = input.tellg();
    input.seekg(0, std::ios::end);
    const auto payloadEnd = input.tellg();
    constexpr std::size_t encodedBlockSize = 128;
    if (payloadStart < 0 || payloadEnd < payloadStart ||
        static_cast<std::uint64_t>(payloadEnd - payloadStart) / encodedBlockSize != blockCount ||
        (payloadEnd - payloadStart) % encodedBlockSize != 0) return false;
    input.seekg(payloadStart);

    const ElGamal cipher(privateKey);
    Bytes output;
    output.reserve(size);

    for (std::size_t block = 0; block < blockCount; ++block) {
        CipherBlock ciphertext;
        if (!readPoint(input, ciphertext.c1) || !readPoint(input, ciphertext.c2)) return false;

        CurvePoint message;
        if (!cipher.decrypt(ciphertext, message)) return false;
        std::array<Byte, ElGamal::BlockSize> decoded{};
        if (!ElGamal::pointMessage(message, decoded)) return false;

        const std::size_t blockSize = std::min(ElGamal::BlockSize, size - block * ElGamal::BlockSize);
        output.insert(output.end(), decoded.end() - static_cast<std::ptrdiff_t>(blockSize), decoded.end());
    }

    Byte trailing = 0;
    if (input.read(reinterpret_cast<char*>(&trailing), 1)) return false;
    if (!input.eof() || output.size() != size) return false;

    std::ofstream result(outputPath, std::ios::binary | std::ios::trunc);
    if (!result) return false;
    if (!writeBytes(result, output.data(), output.size())) return false;
    result.flush();
    return result.good();
}

bool CipherFile::readPrivateKey(const std::string& path, UInt256& privateKey)
{
    std::vector<UInt256> fields;
    if (!parseFields(path, {"d"}, fields) || fields[0].zero() ||
        UInt256::compare(fields[0], EllipticCurve::order()) >= 0) return false;
    privateKey = fields[0];
    return true;
}

bool CipherFile::readPublicKey(const std::string& path, CurvePoint& publicKey)
{
    std::vector<UInt256> fields;
    if (!parseFields(path, {"x", "y"}, fields) ||
        !EllipticCurve::validPublicKey(fields[0], fields[1])) return false;
    publicKey = EllipticCurve::affinePoint(fields[0], fields[1]);
    return true;
}

bool CipherFile::parseFields(const std::string& path,
                           const std::vector<std::string>& names,
                           std::vector<UInt256>& values)
{
    std::ifstream input(path);
    if (!input) return false;
    values.clear();
    std::string line;
    for (const auto& name : names) {
        if (!std::getline(input, line)) return false;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.compare(0, name.size() + 1, name + "=") != 0) return false;
        UInt256 value;
        if (!UInt256::fromHex(line.substr(name.size() + 1), value)) return false;
        values.push_back(value);
    }
    return !std::getline(input, line);
}

bool CipherFile::writeFields(const std::string& path,
                           const std::vector<std::pair<std::string, UInt256>>& fields)
{
    std::ofstream output(path, std::ios::trunc);
    if (!output) return false;
    for (const auto& field : fields) output << field.first << '=' << UInt256::toHex(field.second) << '\n';
    output.flush();
    return output.good();
}

bool CipherFile::generateKeys(const std::string& privatePath, const std::string& publicPath)
{
    if (privatePath == publicPath) return false;

    UInt256 privateKey;
    UInt256 publicX;
    UInt256 publicY;
    CurvePoint publicKey;
    if (!ElGamal::generateKeyPair(privateKey, publicKey) ||
        !EllipticCurve::toAffine(publicKey, publicX, publicY)) {
        return false;
    }

    return writeFields(privatePath, {{"d", privateKey}}) &&
           writeFields(publicPath, {{"x", publicX}, {"y", publicY}});
}
