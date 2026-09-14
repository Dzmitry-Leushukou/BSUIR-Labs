#include "GStructures.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using Byte = std::uint8_t;
using ByteVector = std::vector<Byte>;

std::uint32_t substitute(std::uint32_t value)
{
    std::uint32_t result = 0;
    for (std::size_t sBoxIndex = 0; sBoxIndex < 8; ++sBoxIndex)
    {
        const std::uint32_t inputValue = (value >> (4 * sBoxIndex)) & 0x0F;
        result |= static_cast<std::uint32_t>(GSTable[sBoxIndex][inputValue]) << (4 * sBoxIndex);
    }
    return result;
}

std::uint32_t rotateLeft(std::uint32_t value, unsigned shift)
{
    return (value << shift) | (value >> (32 - shift));
}

std::uint32_t roundFunction(std::uint32_t rightPart, std::uint32_t roundKey)
{
    return rotateLeft(substitute(rightPart + roundKey), 11);
}

GSubKeys createEncryptionSubKeys(const GKey& key)
{
    GSubKeys subKeys{};
    std::size_t subKeyIndex = 0;
    for (std::size_t repetition = 0; repetition < 3; ++repetition)
    {
        for (std::size_t keyIndex = 0; keyIndex < key.size(); ++keyIndex)
        {
            subKeys[subKeyIndex++] = key[keyIndex];
        }
    }
    for (int keyIndex = static_cast<int>(key.size()) - 1; keyIndex >= 0; --keyIndex)
    {
        subKeys[subKeyIndex++] = key[static_cast<std::size_t>(keyIndex)];
    }
    return subKeys;
}

GSubKeys createDecryptionSubKeys(const GKey& key)
{
    GSubKeys subKeys = createEncryptionSubKeys(key);
    std::reverse(subKeys.begin(), subKeys.end());
    return subKeys;
}

GBlock64 transformBlock(const GBlock64& block, const GSubKeys& subKeys)
{
    std::uint32_t leftPart = block.n1;
    std::uint32_t rightPart = block.n2;
    for (std::size_t round = 0; round < 31; ++round)
    {
        const std::uint32_t nextLeftPart = rightPart;
        const std::uint32_t nextRightPart = leftPart ^ roundFunction(rightPart, subKeys[round]);
        leftPart = nextLeftPart;
        rightPart = nextRightPart;
    }
    leftPart ^= roundFunction(rightPart, subKeys[31]);
    return { leftPart, rightPart };
}

GBlock64 encryptBlock(const GBlock64& block, const GKey& key)
{
    return transformBlock(block, createEncryptionSubKeys(key));
}

GBlock64 decryptBlock(const GBlock64& block, const GKey& key)
{
    return transformBlock(block, createDecryptionSubKeys(key));
}

std::uint32_t readUint32LittleEndian(const Byte* bytes)
{
    return static_cast<std::uint32_t>(bytes[0]) |
           (static_cast<std::uint32_t>(bytes[1]) << 8) |
           (static_cast<std::uint32_t>(bytes[2]) << 16) |
           (static_cast<std::uint32_t>(bytes[3]) << 24);
}

void writeUint32LittleEndian(Byte* bytes, std::uint32_t value)
{
    bytes[0] = static_cast<Byte>(value);
    bytes[1] = static_cast<Byte>(value >> 8);
    bytes[2] = static_cast<Byte>(value >> 16);
    bytes[3] = static_cast<Byte>(value >> 24);
}

GBlock64 bytesToBlock(const Byte* bytes)
{
    return { readUint32LittleEndian(bytes), readUint32LittleEndian(bytes + 4) };
}

std::array<Byte, 8> blockToBytes(const GBlock64& block)
{
    std::array<Byte, 8> bytes{};
    writeUint32LittleEndian(bytes.data(), block.n1);
    writeUint32LittleEndian(bytes.data() + 4, block.n2);
    return bytes;
}

ByteVector readBinaryFile(const std::string& filePath)
{
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile) throw std::runtime_error("Cannot open input file: " + filePath);
    return { std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>() };
}

void writeBinaryFile(const std::string& filePath, const ByteVector& data)
{
    std::ofstream outputFile(filePath, std::ios::binary);
    if (!outputFile) throw std::runtime_error("Cannot open output file: " + filePath);
    outputFile.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

ByteVector addPadding(const ByteVector& data)
{
    ByteVector paddedData = data;
    const Byte paddingSize = static_cast<Byte>(8 - paddedData.size() % 8);
    paddedData.insert(paddedData.end(), paddingSize, paddingSize);
    return paddedData;
}

ByteVector removePadding(const ByteVector& data)
{
    if (data.empty() || data.size() % 8 != 0) throw std::runtime_error("Invalid decrypted data length");
    const Byte paddingSize = data.back();
    if (paddingSize == 0 || paddingSize > 8 || paddingSize > data.size()) throw std::runtime_error("Invalid final block padding");
    if (!std::all_of(data.end() - paddingSize, data.end(), [paddingSize](Byte value) { return value == paddingSize; })) throw std::runtime_error("Invalid final block padding");
    return ByteVector(data.begin(), data.end() - paddingSize);
}

ByteVector encryptBlocks(const ByteVector& data, const GKey& key)
{
    const ByteVector paddedData = addPadding(data);
    ByteVector encryptedData(paddedData.size());
    for (std::size_t offset = 0; offset < paddedData.size(); offset += 8)
    {
        const auto encryptedBlock = blockToBytes(encryptBlock(bytesToBlock(paddedData.data() + offset), key));
        std::copy(encryptedBlock.begin(), encryptedBlock.end(), encryptedData.begin() + static_cast<std::ptrdiff_t>(offset));
    }
    return encryptedData;
}

ByteVector decryptBlocks(const ByteVector& data, const GKey& key)
{
    if (data.empty() || data.size() % 8 != 0) throw std::runtime_error("Ciphertext length must be a multiple of 8 bytes");
    ByteVector decryptedData(data.size());
    for (std::size_t offset = 0; offset < data.size(); offset += 8)
    {
        const auto decryptedBlock = blockToBytes(decryptBlock(bytesToBlock(data.data() + offset), key));
        std::copy(decryptedBlock.begin(), decryptedBlock.end(), decryptedData.begin() + static_cast<std::ptrdiff_t>(offset));
    }
    return removePadding(decryptedData);
}

void incrementCounter(GBlock64& counter)
{
    ++counter.n1;
    if (counter.n1 == 0) ++counter.n2;
}

ByteVector gammaCrypt(const ByteVector& data, const GKey& key, GBlock64 counter)
{
    ByteVector result(data.size());
    for (std::size_t offset = 0; offset < data.size(); offset += 8)
    {
        const auto gammaBytes = blockToBytes(encryptBlock(counter, key));
        const std::size_t bytesToProcess = std::min<std::size_t>(8, data.size() - offset);
        for (std::size_t byteIndex = 0; byteIndex < bytesToProcess; ++byteIndex) result[offset + byteIndex] = data[offset + byteIndex] ^ gammaBytes[byteIndex];
        incrementCounter(counter);
    }
    return result;
}

ByteVector feedbackCrypt(const ByteVector& data, const GKey& key, GBlock64 feedback, bool decrypting)
{
    ByteVector result(data.size());
    for (std::size_t offset = 0; offset < data.size(); offset += 8)
    {
        const auto gammaBytes = blockToBytes(encryptBlock(feedback, key));
        const std::size_t bytesToProcess = std::min<std::size_t>(8, data.size() - offset);
        std::array<Byte, 8> nextFeedbackBytes{};
        for (std::size_t byteIndex = 0; byteIndex < bytesToProcess; ++byteIndex)
        {
            result[offset + byteIndex] = data[offset + byteIndex] ^ gammaBytes[byteIndex];
            nextFeedbackBytes[byteIndex] = decrypting ? data[offset + byteIndex] : result[offset + byteIndex];
        }
        feedback = bytesToBlock(nextFeedbackBytes.data());
    }
    return result;
}

std::array<Byte, 4> calculateImito(const ByteVector& data, const GKey& key)
{
    ByteVector paddedData = data;
    if (paddedData.empty() || paddedData.size() % 8 != 0) paddedData.resize((paddedData.size() + 7) / 8 * 8, 0);
    GBlock64 state{ 0, 0 };
    for (std::size_t offset = 0; offset < paddedData.size(); offset += 8)
    {
        const GBlock64 messageBlock = bytesToBlock(paddedData.data() + offset);
        state.n1 ^= messageBlock.n1;
        state.n2 ^= messageBlock.n2;
        state = encryptBlock(state, key);
    }
    const auto stateBytes = blockToBytes(state);
    return { stateBytes[0], stateBytes[1], stateBytes[2], stateBytes[3] };
}

GKey parseKey(const std::string& keyText)
{
    if (keyText.size() != 64) throw std::invalid_argument("Key must contain 64 hexadecimal characters");
    GKey key{};
    for (std::size_t wordIndex = 0; wordIndex < key.size(); ++wordIndex)
    {
        std::uint32_t word = 0;
        for (std::size_t characterIndex = 0; characterIndex < 8; ++characterIndex)
        {
            const char character = keyText[wordIndex * 8 + characterIndex];
            word <<= 4;
            if (character >= '0' && character <= '9') word |= static_cast<std::uint32_t>(character - '0');
            else if (character >= 'a' && character <= 'f') word |= static_cast<std::uint32_t>(character - 'a' + 10);
            else if (character >= 'A' && character <= 'F') word |= static_cast<std::uint32_t>(character - 'A' + 10);
            else throw std::invalid_argument("Key contains an invalid character");
        }
        key[wordIndex] = word;
    }
    return key;
}

GBlock64 parseBlock(const std::string& blockText)
{
    if (blockText.size() != 16) throw std::invalid_argument("Initialization vector must contain 16 hexadecimal characters");
    std::array<Byte, 8> bytes{};
    for (std::size_t byteIndex = 0; byteIndex < bytes.size(); ++byteIndex) bytes[byteIndex] = static_cast<Byte>(std::stoul(blockText.substr(byteIndex * 2, 2), nullptr, 16));
    return bytesToBlock(bytes.data());
}

std::string toHex(const std::array<Byte, 4>& bytes)
{
    std::ostringstream result;
    result << std::hex << std::setfill('0');
    for (Byte byte : bytes) result << std::setw(2) << static_cast<unsigned>(byte);
    return result.str();
}

void printUsage(const char* programName)
{
    std::cerr << "Usage:\n"
              << programName << " encrypt simple <input> <output> <key64>\n"
              << programName << " decrypt simple <input> <output> <key64>\n"
              << programName << " encrypt gamma <input> <output> <key64> <iv16>\n"
              << programName << " decrypt gamma <input> <output> <key64> <iv16>\n"
              << programName << " encrypt feedback <input> <output> <key64> <iv16>\n"
              << programName << " decrypt feedback <input> <output> <key64> <iv16>\n"
              << programName << " imito <input> <key64>\n";
}

int main(int argumentCount, char* argumentValues[])
{
    try
    {
        if (argumentCount < 2) { printUsage(argumentValues[0]); return 1; }
        const std::string operation = argumentValues[1];
        if (operation == "imito")
        {
            if (argumentCount != 4) { printUsage(argumentValues[0]); return 1; }
            std::cout << toHex(calculateImito(readBinaryFile(argumentValues[2]), parseKey(argumentValues[3]))) << '\n';
            return 0;
        }
        if (argumentCount < 6) { printUsage(argumentValues[0]); return 1; }
        const std::string mode = argumentValues[2];
        const GKey key = parseKey(argumentValues[5]);
        const ByteVector inputData = readBinaryFile(argumentValues[3]);
        ByteVector outputData;
        if (mode == "simple")
        {
            if (operation == "encrypt") outputData = encryptBlocks(inputData, key);
            else if (operation == "decrypt") outputData = decryptBlocks(inputData, key);
            else throw std::invalid_argument("Operation must be encrypt or decrypt");
        }
        else
        {
            if (argumentCount != 7) { printUsage(argumentValues[0]); return 1; }
            const GBlock64 synchronizationValue = parseBlock(argumentValues[6]);
            if (operation != "encrypt" && operation != "decrypt") throw std::invalid_argument("Operation must be encrypt or decrypt");
            if (mode == "gamma") outputData = gammaCrypt(inputData, key, synchronizationValue);
            else if (mode == "feedback") outputData = feedbackCrypt(inputData, key, synchronizationValue, operation == "decrypt");
            else throw std::invalid_argument("Unknown operation mode");
        }
        writeBinaryFile(argumentValues[4], outputData);
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
