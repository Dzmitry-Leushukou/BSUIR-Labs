#include "crypto.hpp"

#include <iostream>

namespace {

void printUsage(const char* program)
{
    std::cerr << "Usage:\n"
              << "  " << program << " keygen <private_key_file> <public_key_file>\n"
              << "  " << program << " sign <private_key_file> <input_file> <signature_file>\n"
              << "  " << program << " verify <public_key_file> <input_file> <signature_file>\n";
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc == 4 && std::string(argv[1]) == "keygen") {
        if (!gost::generateKeys(argv[2], argv[3])) {
            std::cerr << "Error: key generation failed\n";
            return 1;
        }
        std::cout << "Key pair generated\n";
        return 0;
    }

    if (argc == 5 && std::string(argv[1]) == "sign") {
        if (!gost::signFile(argv[2], argv[3], argv[4])) {
            std::cerr << "Error: signature generation failed\n";
            return 1;
        }
        std::cout << "Signature created\n";
        return 0;
    }

    if (argc == 5 && std::string(argv[1]) == "verify") {
        if (!gost::verifyFile(argv[2], argv[3], argv[4])) {
            std::cerr << "Signature verification failed\n";
            return 2;
        }
        std::cout << "Signature is valid\n";
        return 0;
    }

    printUsage(argv[0]);
    return 1;
}
