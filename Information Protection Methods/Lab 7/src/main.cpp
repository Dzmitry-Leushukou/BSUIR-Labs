#include "CipherFile.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc == 4 && std::string(argv[1]) == "keygen") {
        if (!CipherFile::generateKeys(argv[2], argv[3])) {
            std::cerr << "Error: key generation failed\n";
            return 1;
        }
        std::cout << "Key pair generated\n";
        return 0;
    }

    if (argc == 5 && std::string(argv[1]) == "encrypt") {
        if (!CipherFile::encryptFile(argv[2], argv[3], argv[4])) {
            std::cerr << "Error: encryption failed\n";
            return 1;
        }
        std::cout << "File encrypted\n";
        return 0;
    }

    if (argc == 5 && std::string(argv[1]) == "decrypt") {
        if (!CipherFile::decryptFile(argv[2], argv[3], argv[4])) {
            std::cerr << "Error: decryption failed\n";
            return 1;
        }
        std::cout << "File decrypted\n";
        return 0;
    }

    std::cerr << "Usage:\n"
              << "  " << argv[0] << " keygen <private_key_file> <public_key_file>\n"
              << "  " << argv[0] << " encrypt <public_key_file> <input_file> <encrypted_file>\n"
              << "  " << argv[0] << " decrypt <private_key_file> <encrypted_file> <output_file>\n";
    return 1;
}
