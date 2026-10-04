#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <cstdint>
#include <algorithm>
#include <stdexcept>

#include "BigInt.h"
#include "PublicKey.h"
#include "PrivateKey.h"

using Byte = std::uint8_t;
using ByteVector = std::vector<Byte>;

std::mt19937_64& getRandomEngine() {
    static std::mt19937_64 generator(std::random_device{}());
    return generator;
}

Byte randomByte() {
    return static_cast<Byte>(getRandomEngine()() & 0xFF);
}

BigInt randomBits(unsigned bitCount) {
    BigInt value = 0;
    const unsigned fullBytes = bitCount / 8;
    const unsigned remainingBits = bitCount % 8;

    for (unsigned i = 0; i < fullBytes; ++i) {
        value <<= 8;
        value += BigInt(randomByte());
    }

    if (remainingBits != 0) {
        value <<= remainingBits;
        Byte lastByte = randomByte();
        lastByte &= (1 << remainingBits) - 1;
        value += BigInt(lastByte);
    }

    value.setBit(bitCount - 1);
    value.setBit(0);

    return value;
}

BigInt modPow(BigInt base, BigInt exponent, const BigInt& modulus) {
    BigInt result = 1;
    base = base % modulus;

    while (exponent > BigInt(0)) {
        if ((exponent & BigInt(1)) != BigInt(0)) {
            result = (result * base) % modulus;
        }

        base = (base * base) % modulus;
        exponent >>= 1;
    }

    return result;
}

BigInt gcd(BigInt a, BigInt b) {
    while (b != BigInt(0)) {
        BigInt remainder = a % b;
        a = b;
        b = remainder;
    }

    return a;
}

BigInt extendedGcd(const BigInt& a, const BigInt& b, BigInt& x, BigInt& y) {
    if (b == BigInt(0)) {
        x = 1;
        y = 0;
        return a;
    }

    BigInt x1, y1;
    BigInt result = extendedGcd(b, a % b, x1, y1);

    x = y1;
    y = x1 - (a / b) * y1;

    return result;
}

bool isProbablePrime(const BigInt& value, int rounds = 32) {
    static constexpr int smallPrimes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
    };

    for (const int prime : smallPrimes) {
        if (value == BigInt(prime)) {
            return true;
        }

        if ((value % BigInt(prime)) == BigInt(0)) {
            return false;
        }
    }

    if (value < BigInt(2) || (value & BigInt(1)) == BigInt(0)) {
        return false;
    }

    BigInt d = value - BigInt(1);
    unsigned powerOfTwo = 0;

    while ((d & BigInt(1)) == BigInt(0)) {
        d >>= 1;
        ++powerOfTwo;
    }

    for (int round = 0; round < rounds; ++round) {
        BigInt base = BigInt(2) + BigInt(getRandomEngine()() % 100);
        if (base >= value - BigInt(2)) {
            base = BigInt(2);
        }

        BigInt x = modPow(base, d, value);

        if (x == BigInt(1) || x == value - BigInt(1)) {
            continue;
        }

        bool passed = false;

        for (unsigned i = 1; i < powerOfTwo; ++i) {
            x = (x * x) % value;

            if (x == value - BigInt(1)) {
                passed = true;
                break;
            }
        }

        if (!passed) {
            return false;
        }
    }

    return true;
}

BigInt generateBlumPrime(unsigned bitCount) {
    while (true) {
        BigInt candidate = randomBits(bitCount);

        BigInt remainder = candidate % BigInt(4);
        if (remainder == BigInt(1)) {
            candidate = candidate + BigInt(2);
        } else if (remainder == BigInt(0) || remainder == BigInt(2)) {
            candidate = candidate + BigInt(3);
        }

        if (isProbablePrime(candidate)) {
            return candidate;
        }
    }
}

BigInt bytesToInteger(const ByteVector& bytes) {
    BigInt value = 0;

    for (const Byte byte : bytes) {
        value <<= 8;
        value += BigInt(byte);
    }

    return value;
}

ByteVector integerToBytes(BigInt value, std::size_t size) {
    ByteVector bytes(size, 0);

    for (std::size_t i = size; i-- > 0;) {
        BigInt remainder = value % BigInt(256);
        bytes[i] = static_cast<Byte>(std::stoi(remainder.toString()));
        value = value / BigInt(256);
    }

    return bytes;
}

void writeUInt32(std::ostream& stream, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        stream.put(static_cast<char>((value >> (i * 8)) & 0xFF));
    }
}

std::uint32_t readUInt32(std::istream& stream) {
    std::uint32_t value = 0;

    for (int i = 0; i < 4; ++i) {
        const int byte = stream.get();

        if (byte == EOF) {
            throw std::runtime_error("Unexpected end of file.");
        }

        value |= static_cast<std::uint32_t>(static_cast<Byte>(byte)) << (i * 8);
    }

    return value;
}

std::uint64_t readUInt64(std::istream& stream) {
    std::uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        const int byte = stream.get();

        if (byte == EOF) {
            throw std::runtime_error("Unexpected end of file.");
        }

        value |= static_cast<std::uint64_t>(static_cast<Byte>(byte)) << (i * 8);
    }

    return value;
}

void writeUInt64(std::ostream& stream, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        stream.put(static_cast<char>((value >> (i * 8)) & 0xFF));
    }
}

std::uint32_t calculateCrc32(const ByteVector& data) {
    std::uint32_t crc = 0xFFFFFFFFu;

    for (const Byte byte : data) {
        crc ^= byte;

        for (int i = 0; i < 8; ++i) {
            const std::uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;
}

void generateKeys(unsigned primeBitCount, const std::string& publicPath, const std::string& privatePath) {
    std::cout << "Generating " << primeBitCount << "-bit prime numbers...\n";

    BigInt p = generateBlumPrime(primeBitCount);
    BigInt q;

    do {
        q = generateBlumPrime(primeBitCount);
    } while (p == q);

    PublicKey publicKey;
    publicKey.n = p * q;

    PrivateKey privateKey(p, q);

    publicKey.save(publicPath);
    privateKey.save(privatePath);

    const unsigned modulusBits = publicKey.n.bitLength();

    std::cout << "Key generation completed.\n";
    std::cout << "Public key:  " << publicPath << '\n';
    std::cout << "Private key: " << privatePath << '\n';
    std::cout << "Modulus size: " << modulusBits << " bits\n";
}

constexpr std::size_t BLOCK_HEADER_SIZE = 16;
constexpr std::size_t BLOCK_CRC_SIZE = 4;
constexpr char BLOCK_MAGIC[] = {'R', 'B', 'N', '1'};

ByteVector encodeBlock(const ByteVector& data, std::size_t offset, std::size_t size, std::size_t blockSize) {
    ByteVector block(blockSize, 0);

    std::copy(std::begin(BLOCK_MAGIC), std::end(BLOCK_MAGIC), block.begin());

    const auto dataSize = static_cast<std::uint32_t>(size);

    for (int i = 0; i < 4; ++i) {
        block[4 + i] = static_cast<Byte>((dataSize >> (i * 8)) & 0xFF);
    }

    for (int i = 0; i < 8; ++i) {
        block[8 + i] = randomByte();
    }

    std::copy(data.begin() + static_cast<std::ptrdiff_t>(offset),
              data.begin() + static_cast<std::ptrdiff_t>(offset + size),
              block.begin() + BLOCK_HEADER_SIZE);

    ByteVector payload(
        data.begin() + static_cast<std::ptrdiff_t>(offset),
        data.begin() + static_cast<std::ptrdiff_t>(offset + size)
    );

    const std::uint32_t checksum = calculateCrc32(payload);
    const std::size_t checksumOffset = BLOCK_HEADER_SIZE + size;

    for (int i = 0; i < 4; ++i) {
        block[checksumOffset + i] =
            static_cast<Byte>((checksum >> (i * 8)) & 0xFF);
    }

    return block;
}

bool decodeBlock(const ByteVector& block, ByteVector& data) {
    if (block.size() < BLOCK_HEADER_SIZE + BLOCK_CRC_SIZE) {
        return false;
    }

    if (!std::equal(std::begin(BLOCK_MAGIC), std::end(BLOCK_MAGIC), block.begin())) {
        return false;
    }

    std::uint32_t dataSize = 0;

    for (int i = 0; i < 4; ++i) {
        dataSize |= static_cast<std::uint32_t>(block[4 + i]) << (i * 8);
    }

    if (dataSize > block.size() - BLOCK_HEADER_SIZE - BLOCK_CRC_SIZE) {
        return false;
    }

    const std::size_t checksumOffset = BLOCK_HEADER_SIZE + dataSize;
    std::uint32_t storedChecksum = 0;

    for (int i = 0; i < 4; ++i) {
        storedChecksum |=
            static_cast<std::uint32_t>(block[checksumOffset + i]) << (i * 8);
    }

    data.assign(
        block.begin() + BLOCK_HEADER_SIZE,
        block.begin() + BLOCK_HEADER_SIZE + dataSize
    );

    return calculateCrc32(data) == storedChecksum;
}

BigInt rabinEncrypt(const BigInt& message, const PublicKey& key) {
    return (message * message) % key.n;
}

ByteVector rabinDecrypt(const BigInt& ciphertext, const PrivateKey& key, std::size_t blockSize) {
    BigInt pRoot = modPow(
        ciphertext % key.p,
        (key.p + BigInt(1)) / BigInt(4),
        key.p
    );

    BigInt qRoot = modPow(
        ciphertext % key.q,
        (key.q + BigInt(1)) / BigInt(4),
        key.q
    );

    BigInt pCoefficient, qCoefficient;
    extendedGcd(key.p, key.q, pCoefficient, qCoefficient);

    BigInt root1 =
        (pCoefficient * key.p * qRoot +
         qCoefficient * key.q * pRoot) %
        key.n;

    if (root1 < BigInt(0)) {
        root1 += key.n;
    }

    BigInt root2 = key.n - root1;

    BigInt root3 =
        (pCoefficient * key.p * qRoot -
         qCoefficient * key.q * pRoot) %
        key.n;

    if (root3 < BigInt(0)) {
        root3 += key.n;
    }

    BigInt root4 = key.n - root3;

    const std::vector<BigInt> roots = {root1, root2, root3, root4};

    for (const BigInt& root : roots) {
        ByteVector block = integerToBytes(root, blockSize);
        ByteVector data;

        if (decodeBlock(block, data)) {
            return block;
        }
    }

    throw std::runtime_error(
        "No valid plaintext root was found. The key or ciphertext may be corrupted."
    );
}

ByteVector readFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);

    if (!input) {
        throw std::runtime_error("Cannot open input file.");
    }

    return ByteVector(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    );
}

void writeEncryptedBlock(std::ofstream& output, const ByteVector& block, const PublicKey& key, std::size_t modulusBytes) {
    const BigInt message = bytesToInteger(block);
    const BigInt ciphertext = rabinEncrypt(message, key);

    const ByteVector encrypted = integerToBytes(ciphertext, modulusBytes);

    output.write(
        reinterpret_cast<const char*>(encrypted.data()),
        static_cast<std::streamsize>(encrypted.size())
    );
}

void encryptFile(const std::string& inputPath, const std::string& outputPath, const std::string& publicKeyPath) {
    const PublicKey key = PublicKey::load(publicKeyPath);
    const ByteVector data = readFile(inputPath);

    const std::size_t modulusBytes = (key.n.bitLength() + 7) / 8;
    const std::size_t blockSize = modulusBytes - 1;

    if (blockSize <= BLOCK_HEADER_SIZE + BLOCK_CRC_SIZE) {
        throw std::runtime_error("The modulus is too small.");
    }

    const std::size_t maxPayloadSize = blockSize - BLOCK_HEADER_SIZE - BLOCK_CRC_SIZE;

    std::ofstream output(outputPath, std::ios::binary);

    if (!output) {
        throw std::runtime_error("Cannot create ciphertext file.");
    }

    output.write("RBN1", 4);
    writeUInt32(output, static_cast<std::uint32_t>(blockSize));
    writeUInt64(output, static_cast<std::uint64_t>(data.size()));

    if (data.empty()) {
        const ByteVector block = encodeBlock(data, 0, 0, blockSize);
        writeEncryptedBlock(output, block, key, modulusBytes);
    } else {
        for (std::size_t offset = 0; offset < data.size();) {
            const std::size_t payloadSize = std::min(maxPayloadSize, data.size() - offset);

            const ByteVector block = encodeBlock(data, offset, payloadSize, blockSize);

            writeEncryptedBlock(output, block, key, modulusBytes);
            offset += payloadSize;
        }
    }

    std::cout << "Encryption completed.\n";
    std::cout << "Input size: " << data.size() << " bytes\n";
    std::cout << "Output file: " << outputPath << '\n';
}

void decryptFile(const std::string& inputPath, const std::string& outputPath, const std::string& privateKeyPath) {
    const PrivateKey key = PrivateKey::load(privateKeyPath);

    std::ifstream input(inputPath, std::ios::binary);

    if (!input) {
        throw std::runtime_error("Cannot open ciphertext file.");
    }

    char header[4];
    input.read(header, 4);

    if (input.gcount() != 4 || std::string(header, 4) != "RBN1") {
        throw std::runtime_error("Invalid ciphertext format.");
    }

    const std::uint32_t blockSize = readUInt32(input);
    const std::uint64_t originalSize = readUInt64(input);

    const std::size_t modulusBytes = (key.n.bitLength() + 7) / 8;

    if (blockSize != modulusBytes - 1) {
        throw std::runtime_error("Ciphertext and private key do not match.");
    }

    std::ofstream output(outputPath, std::ios::binary);

    if (!output) {
        throw std::runtime_error("Cannot create decrypted file.");
    }

    std::uint64_t written = 0;

    while (input.peek() != EOF) {
        ByteVector encryptedBlock(modulusBytes);
        input.read(
            reinterpret_cast<char*>(encryptedBlock.data()),
            static_cast<std::streamsize>(encryptedBlock.size())
        );

        if (input.gcount() != static_cast<std::streamsize>(encryptedBlock.size())) {
            throw std::runtime_error("Incomplete ciphertext block.");
        }

        const BigInt ciphertext = bytesToInteger(encryptedBlock);

        if (ciphertext >= key.n) {
            throw std::runtime_error("Invalid ciphertext block.");
        }

        const ByteVector block = rabinDecrypt(ciphertext, key, blockSize);

        ByteVector plaintext;

        if (!decodeBlock(block, plaintext)) {
            throw std::runtime_error("Invalid plaintext block.");
        }

        const std::uint64_t remaining = originalSize - written;
        const std::size_t bytesToWrite =
            static_cast<std::size_t>(
                std::min<std::uint64_t>(plaintext.size(), remaining)
            );

        output.write(
            reinterpret_cast<const char*>(plaintext.data()),
            static_cast<std::streamsize>(bytesToWrite)
        );

        written += bytesToWrite;
    }

    if (written != originalSize) {
        throw std::runtime_error(
            "Decrypted file size does not match the original size."
        );
    }

    std::cout << "Decryption completed.\n";
    std::cout << "Output size: " << written << " bytes\n";
    std::cout << "Output file: " << outputPath << '\n';
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n";
        std::cerr << "  Generate keys: " << argv[0] << " keygen <bits> <public_key> <private_key>\n";
        std::cerr << "  Encrypt file:  " << argv[0] << " encrypt <input> <output> <public_key>\n";
        std::cerr << "  Decrypt file:  " << argv[0] << " decrypt <input> <output> <private_key>\n";
        return 1;
    }

    try {
        std::string mode = argv[1];

        if (mode == "keygen") {
            if (argc != 5) {
                std::cerr << "Usage: " << argv[0] << " keygen <bits> <public_key> <private_key>\n";
                return 1;
            }

            unsigned bits = std::stoi(argv[2]);
            std::string publicPath = argv[3];
            std::string privatePath = argv[4];

            if (bits != 256 && bits != 512) {
                std::cerr << "Error: only 256 or 512 bits are supported\n";
                return 1;
            }

            generateKeys(bits, publicPath, privatePath);
        }
        else if (mode == "encrypt") {
            if (argc != 5) {
                std::cerr << "Usage: " << argv[0] << " encrypt <input> <output> <public_key>\n";
                return 1;
            }

            std::string inputPath = argv[2];
            std::string outputPath = argv[3];
            std::string keyPath = argv[4];

            encryptFile(inputPath, outputPath, keyPath);
        }
        else if (mode == "decrypt") {
            if (argc != 5) {
                std::cerr << "Usage: " << argv[0] << " decrypt <input> <output> <private_key>\n";
                return 1;
            }

            std::string inputPath = argv[2];
            std::string outputPath = argv[3];
            std::string keyPath = argv[4];

            decryptFile(inputPath, outputPath, keyPath);
        }
        else {
            std::cerr << "Error: unknown mode '" << mode << "'\n";
            std::cerr << "Valid modes: keygen, encrypt, decrypt\n";
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
