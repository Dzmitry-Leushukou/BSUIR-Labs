#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

#include "STBStructures.h"

constexpr KTABLE ktable={
0xB1, 0x94, 0xBA, 0xC8, 0x0A, 0x08, 0xF5, 0x3B, 0x36, 0x6D, 0x00, 0x8E, 0x58, 0x4A, 0x5D, 0xE4,
0x85, 0x04, 0xFA, 0x9D, 0x1B, 0xB6, 0xC7, 0xAC, 0x25, 0x2E, 0x72, 0xC2, 0x02, 0xFD, 0xCE, 0x0D,
0x5B, 0xE3, 0xD6, 0x12, 0x17, 0xB9, 0x61, 0x81, 0xFE, 0x67, 0x86, 0xAD, 0x71, 0x6B, 0x89, 0x0B,
0x5C, 0xB0, 0xC0, 0xFF, 0x33, 0xC3, 0x56, 0xB8, 0x35, 0xC4, 0x05, 0xAE, 0xD8, 0xE0, 0x7F, 0x99,
0xE1, 0x2B, 0xDC, 0x1A, 0xE2, 0x82, 0x57, 0xEC, 0x70, 0x3F, 0xCC, 0xF0, 0x95, 0xEE, 0x8D, 0xF1,
0xC1, 0xAB, 0x76, 0x38, 0x9F, 0xE6, 0x78, 0xCA, 0xF7, 0xC6, 0xF8, 0x60, 0xD5, 0xBB, 0x9C, 0x4F,
0xF3, 0x3C, 0x65, 0x7B, 0x63, 0x7C, 0x30, 0x6A, 0xDD, 0x4E, 0xA7, 0x79, 0x9E, 0xB2, 0x3D, 0x31,
0x3E, 0x98, 0xB5, 0x6E, 0x27, 0xD3, 0xBC, 0xCF, 0x59, 0x1E, 0x18, 0x1F, 0x4C, 0x5A, 0xB7, 0x93,
0xE9, 0xDE, 0xE7, 0x2C, 0x8F, 0x0C, 0x0F, 0xA6, 0x2D, 0xDB, 0x49, 0xF4, 0x6F, 0x73, 0x96, 0x47,
0x06, 0x07, 0x53, 0x16, 0xED, 0x24, 0x7A, 0x37, 0x39, 0xCB, 0xA3, 0x83, 0x03, 0xA9, 0x8B, 0xF6,
0x92, 0xBD, 0x9B, 0x1C, 0xE5, 0xD1, 0x41, 0x01, 0x54, 0x45, 0xFB, 0xC9, 0x5E, 0x4D, 0x0E, 0xF2,
0x68, 0x20, 0x80, 0xAA, 0x22, 0x7D, 0x64, 0x2F, 0x26, 0x87, 0xF9, 0x34, 0x90, 0x40, 0x55, 0x11,
0xBE, 0x32, 0x97, 0x13, 0x43, 0xFC, 0x9A, 0x48, 0xA0, 0x2A, 0x88, 0x5F, 0x19, 0x4B, 0x09, 0xA1,
0x7E, 0xCD, 0xA4, 0xD0, 0x15, 0x44, 0xAF, 0x8C, 0xA5, 0x84, 0x50, 0xBF, 0x66, 0xD2, 0xE8, 0x8A,
0xA2, 0xD7, 0x46, 0x52, 0x42, 0xA8, 0xDF, 0xB3, 0x69, 0x74, 0xC5, 0x51, 0xEB, 0x23, 0x29, 0x21,
0xD4, 0xEF, 0xD9, 0xB4, 0x3A, 0x62, 0x28, 0x75, 0x91, 0x14, 0x10, 0xEA, 0x77, 0x6C, 0xDA, 0x1D
};

KEY key;
SUBKEYS sub_keys;
std::vector<DBLOCK> file_data;
std::size_t data_size = 0;

void substitute(uint32_t& x)
{
    uint8_t a = (x >> 24) & 0xFF;
    uint8_t b = (x >> 16) & 0xFF;
    uint8_t c = (x >> 8) & 0xFF;
    uint8_t d = x & 0xFF;

    x = ((uint32_t)ktable[a] << 8*3) |
        ((uint32_t)ktable[b] << 8*2) |
        ((uint32_t)ktable[c] << 8) |
        ((uint32_t)ktable[d]);
}

void rotl(uint32_t& x, const unsigned r)
{
    uint32_t e = x >> (32 - r);
    x = (x << r) | e;
}

void G(uint32_t& x, const unsigned r)
{
    substitute(x);
    rotl(x, r);
}

uint32_t add_mod(uint32_t x, uint32_t y)
{
    return x + y;
}

uint32_t sub_mod(uint32_t x, uint32_t y)
{
    return x - y;
}

void round(DBLOCK& data, unsigned rn)
{
    uint32_t sk = sub_keys[7 * rn - 7];
    uint32_t tmp = add_mod(data[0], sk);
    G(tmp, 5);
    data[1] ^= tmp;

    sk = sub_keys[7 * rn - 6];
    tmp = add_mod(data[3], sk);
    G(tmp, 21);
    data[2] ^= tmp;

    sk = sub_keys[7 * rn - 5];
    tmp = add_mod(data[1], sk);
    G(tmp, 13);
    data[0] = sub_mod(data[0], tmp);

    sk = sub_keys[7 * rn - 4];
    tmp = add_mod(add_mod(data[1], data[2]), sk);
    G(tmp, 21);
    uint32_t e = tmp ^ static_cast<uint32_t>(rn);

    data[1] = add_mod(data[1], e);
    data[2] = sub_mod(data[2], e);

    sk = sub_keys[7 * rn - 3];
    tmp = add_mod(data[2], sk);
    G(tmp, 13);
    data[3] = add_mod(data[3], tmp);

    sk = sub_keys[7 * rn - 2];
    tmp = add_mod(data[0], sk);
    G(tmp, 21);
    data[1] ^= tmp;

    sk = sub_keys[7 * rn - 1];
    tmp = add_mod(data[3], sk);
    G(tmp, 5);
    data[2] ^= tmp;

    std::swap(data[0], data[1]);
    std::swap(data[2], data[3]);
    std::swap(data[1], data[2]);
}

void encrypt_block(DBLOCK& data)
{
    for (unsigned i = 1; i <= 8; i++)
    {
        round(data, i);
    }

    DBLOCK result = {
        data[1],
        data[3],
        data[0],
        data[2]
    };

    data = result;
}

void decrypt_block(DBLOCK& data)
{
    DBLOCK state = {
        data[2],
        data[0],
        data[3],
        data[1]
    };

    data = state;

    for (unsigned i = 8; i >= 1; i--)
    {
        std::swap(data[1], data[2]);
        std::swap(data[2], data[3]);
        std::swap(data[0], data[1]);

        uint32_t sk = sub_keys[7 * i - 1];
        uint32_t tmp = add_mod(data[3], sk);
        G(tmp, 5);
        data[2] ^= tmp;

        sk = sub_keys[7 * i - 2];
        tmp = add_mod(data[0], sk);
        G(tmp, 21);
        data[1] ^= tmp;

        sk = sub_keys[7 * i - 3];
        tmp = add_mod(data[2], sk);
        G(tmp, 13);
        data[3] = sub_mod(data[3], tmp);

        sk = sub_keys[7 * i - 4];
        tmp = add_mod(add_mod(data[1], data[2]), sk);
        G(tmp, 21);
        uint32_t e = tmp ^ static_cast<uint32_t>(i);

        data[1] = sub_mod(data[1], e);
        data[2] = add_mod(data[2], e);

        sk = sub_keys[7 * i - 5];
        tmp = add_mod(data[1], sk);
        G(tmp, 13);
        data[0] = add_mod(data[0], tmp);

        sk = sub_keys[7 * i - 6];
        tmp = add_mod(data[3], sk);
        G(tmp, 21);
        data[2] ^= tmp;

        sk = sub_keys[7 * i - 7];
        tmp = add_mod(data[0], sk);
        G(tmp, 5);
        data[1] ^= tmp;
    }
}

bool read_file(const std::string& path, bool add_padding, bool allow_partial)
{
    std::ifstream fin(path, std::ios::binary);

    file_data.clear();
    data_size = 0;

    if (!fin)
        return false;

    std::vector<uint8_t> bytes;
    uint8_t byte;

    while (fin.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
    {
        bytes.push_back(byte);
    }

    if (add_padding)
    {
        std::size_t remainder = bytes.size() % 16;
        std::size_t padding_size = 16 - remainder;

        for (std::size_t i = 0; i < padding_size; i++)
        {
            bytes.push_back(static_cast<uint8_t>(padding_size));
        }
    }
    else
    {
        if (!allow_partial)
        {
            if (bytes.empty())
                return false;

            if (bytes.size() % 16 != 0)
                return false;
        }
    }

    data_size = bytes.size();

    if (bytes.empty())
        return true;

    std::size_t byte_index = 0;

    while (byte_index < bytes.size())
    {
        DBLOCK block = {0, 0, 0, 0};

        for (unsigned word = 0; word < 4; word++)
        {
            for (unsigned byte_in_word = 0; byte_in_word < 4; byte_in_word++)
            {
                if (byte_index >= bytes.size())
                    break;

                block[word] |=
                    static_cast<uint32_t>(bytes[byte_index])
                    << (8 * byte_in_word);

                byte_index++;
            }
        }

        file_data.push_back(block);
    }

    return true;
}

bool remove_padding()
{
    if (file_data.empty())
        return false;

    std::vector<uint8_t> bytes;

    for (const auto& block : file_data)
    {
        for (const auto& word : block)
        {
            for (unsigned byte = 0; byte < 4; byte++)
            {
                bytes.push_back(
                    static_cast<uint8_t>((word >> (8 * byte)) & 0xFF)
                );
            }
        }
    }

    if (bytes.empty())
        return false;

    uint8_t padding_size = bytes.back();

    if (padding_size == 0 || padding_size > 16)
        return false;

    if (padding_size > bytes.size())
        return false;

    for (std::size_t i = 0; i < padding_size; i++)
    {
        if (bytes[bytes.size() - 1 - i] != padding_size)
            return false;
    }

    bytes.resize(bytes.size() - padding_size);

    data_size = bytes.size();

    file_data.clear();

    std::size_t byte_index = 0;

    while (byte_index < bytes.size())
    {
        DBLOCK block = {0, 0, 0, 0};

        for (unsigned word = 0; word < 4; word++)
        {
            for (unsigned byte_in_word = 0; byte_in_word < 4; byte_in_word++)
            {
                if (byte_index >= bytes.size())
                    break;

                block[word] |=
                    static_cast<uint32_t>(bytes[byte_index])
                    << (8 * byte_in_word);

                byte_index++;
            }
        }

        file_data.push_back(block);
    }

    return true;
}

void write_file(const std::string& path, std::size_t size)
{
    std::ofstream fout(path, std::ios::binary);

    if (!fout)
        return;

    std::size_t written = 0;

    for (const auto& block : file_data)
    {
        for (const auto& word : block)
        {
            for (unsigned byte = 0; byte < 4; byte++)
            {
                if (written >= size)
                    return;

                uint8_t value =
                    static_cast<uint8_t>((word >> (8 * byte)) & 0xFF);

                fout.write(
                    reinterpret_cast<const char*>(&value),
                    sizeof(value)
                );

                written++;
            }
        }
    }
}

void encrypt_file()
{
    for (auto& i : file_data)
    {
        encrypt_block(i);
    }
}

void decrypt_file()
{
    for (auto& i : file_data)
    {
        decrypt_block(i);
    }
}

void gamma_file(DBLOCK sync, bool encryption)
{
    DBLOCK feedback = sync;

    for (std::size_t i = 0; i < file_data.size(); i++)
    {
        DBLOCK previous_ciphertext = file_data[i];

        DBLOCK gamma = feedback;
        encrypt_block(gamma);

        std::size_t block_size =
            std::min<std::size_t>(16, data_size - i * 16);

        for (std::size_t byte = 0; byte < block_size; byte++)
        {
            std::size_t word = byte / 4;
            std::size_t offset = byte % 4;

            uint8_t value =
                static_cast<uint8_t>(
                    (file_data[i][word] >> (8 * offset)) & 0xFF
                );

            uint8_t gamma_byte =
                static_cast<uint8_t>(
                    (gamma[word] >> (8 * offset)) & 0xFF
                );

            value ^= gamma_byte;

            file_data[i][word] &=
                ~(static_cast<uint32_t>(0xFF) << (8 * offset));

            file_data[i][word] |=
                static_cast<uint32_t>(value) << (8 * offset);
        }

        if (encryption)
            feedback = file_data[i];
        else
            feedback = previous_ciphertext;
    }
}

bool parse_key(const std::string& key_string, KEY& key)
{
    if (key_string.size() != 64)
        return false;

    for (unsigned i = 0; i < 8; i++)
    {
        uint32_t word = 0;

        for (unsigned j = 0; j < 8; j++)
        {
            char c = key_string[i * 8 + j];

            uint8_t value;

            if (c >= '0' && c <= '9')
                value = c - '0';
            else if (c >= 'A' && c <= 'F')
                value = c - 'A' + 10;
            else if (c >= 'a' && c <= 'f')
                value = c - 'a' + 10;
            else
                return false;

            word = (word << 4) | value;
        }

        key[i] = word;
    }

    return true;
}

bool parse_sync(const std::string& sync_string, DBLOCK& sync)
{
    if (sync_string.size() != 32)
        return false;

    for (unsigned i = 0; i < 4; i++)
    {
        uint32_t word = 0;

        for (unsigned j = 0; j < 8; j++)
        {
            char c = sync_string[i * 8 + j];

            uint8_t value;

            if (c >= '0' && c <= '9')
                value = c - '0';
            else if (c >= 'A' && c <= 'F')
                value = c - 'A' + 10;
            else if (c >= 'a' && c <= 'f')
                value = c - 'a' + 10;
            else
                return false;

            word = (word << 4) | value;
        }

        sync[i] = word;
    }

    return true;
}

void expand_key()
{
    for (int i = 0; i < 56; i++)
    {
        sub_keys[i] = key[i % 8];
    }
}

int main(int argc, char* argv[])
{
    if (argc < 6 || argc > 7)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <mode> <operation> <key> [sync] <input_file> <output_file>\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string operation = argv[2];
    std::string key_string = argv[3];

    std::string sync_string;
    std::string input_file;
    std::string output_file;

    if (mode == "simple")
    {
        if (argc != 6)
        {
            std::cerr << "Usage: " << argv[0]
                      << " simple <operation> <key> <input_file> <output_file>\n";
            return 1;
        }

        input_file = argv[4];
        output_file = argv[5];
    }
    else if (mode == "gamma")
    {
        if (argc != 7)
        {
            std::cerr << "Usage: " << argv[0]
                      << " gamma <operation> <key> <sync> <input_file> <output_file>\n";
            return 1;
        }

        sync_string = argv[4];
        input_file = argv[5];
        output_file = argv[6];
    }
    else
    {
        std::cerr << "Error: mode must be 'simple' or 'gamma'\n";
        return 1;
    }

    if (operation != "e" && operation != "d")
    {
        std::cerr << "Error: operation must be 'e' or 'd'\n";
        return 1;
    }

    if (!parse_key(key_string, key))
    {
        std::cerr << "Error: key must contain exactly 64 hexadecimal characters\n";
        return 1;
    }

    expand_key();

    DBLOCK sync = {0, 0, 0, 0};

    if (mode == "gamma")
    {
        if (!parse_sync(sync_string, sync))
        {
            std::cerr << "Error: sync must contain exactly 32 hexadecimal characters\n";
            return 1;
        }
    }

    if (mode == "simple")
    {
        if (operation == "e")
        {
            if (!read_file(input_file, true, false))
            {
                std::cerr << "Error: failed to read input file\n";
                return 1;
            }

            if (file_data.empty())
            {
                std::cerr << "Error: failed to read input file\n";
                return 1;
            }

            encrypt_file();
        }
        else
        {
            if (!read_file(input_file, false, false))
            {
                std::cerr << "Error: failed to read input file\n";
                return 1;
            }

            if (file_data.empty())
            {
                std::cerr << "Error: failed to read input file\n";
                return 1;
            }

            decrypt_file();

            if (!remove_padding())
            {
                std::cerr << "Error: invalid padding or corrupted input file\n";
                return 1;
            }
        }
    }
    else
    {
        if (!read_file(input_file, false, true))
        {
            std::cerr << "Error: failed to read input file\n";
            return 1;
        }

        gamma_file(sync, operation == "e");
    }

    write_file(output_file, data_size);

    return 0;
}