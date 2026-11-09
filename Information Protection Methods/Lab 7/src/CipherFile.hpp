#pragma once

#include "EllipticCurve.hpp"

#include <array>
#include <istream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

class CipherFile {
public:
    static bool generateKeys(const std::string& privatePath, const std::string& publicPath);
    static bool encryptFile(const std::string& publicPath,
                             const std::string& inputPath,
                             const std::string& outputPath);
    static bool decryptFile(const std::string& privatePath,
                             const std::string& inputPath,
                             const std::string& outputPath);

private:
    static bool readPrivateKey(const std::string& path, UInt256& privateKey);
    static bool readPublicKey(const std::string& path, CurvePoint& publicKey);
    static bool parseFields(const std::string& path,
                            const std::vector<std::string>& names,
                            std::vector<UInt256>& values);
    static bool writeFields(const std::string& path,
                            const std::vector<std::pair<std::string, UInt256>>& fields);

    static bool readBytes(std::istream& input, Byte* data, std::size_t size);
    static bool writeBytes(std::ostream& output, const Byte* data, std::size_t size);
    static bool readPoint(std::istream& input, CurvePoint& point);
    static bool writePoint(std::ostream& output, const CurvePoint& point);
    static bool readFile(const std::string& path, Bytes& data);
    static bool writeUInt64(std::ostream& output, std::uint64_t value);
    static bool readUInt64(std::istream& input, std::uint64_t& value);
    static constexpr std::array<char, 8> Magic = {'E', 'C', 'E', 'L', 'G', '1', 0, 0};
};
