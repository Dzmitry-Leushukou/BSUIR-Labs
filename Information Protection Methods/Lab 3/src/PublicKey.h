#pragma once

#include "BigInt.h"
#include <string>
#include <fstream>
#include <stdexcept>

class PublicKey {
public:
    BigInt n;

    PublicKey() = default;

    void save(const std::string& path) const {
        std::ofstream output(path);

        if (!output) {
            throw std::runtime_error("Cannot create public key file.");
        }

        output << "RABIN_PUBLIC_V1\n";
        output << n.toString(16) << '\n';
    }

    static PublicKey load(const std::string& path) {
        std::ifstream input(path);

        if (!input) {
            throw std::runtime_error("Cannot open public key file.");
        }

        std::string header;
        input >> header;

        if (header != "RABIN_PUBLIC_V1") {
            throw std::runtime_error("Invalid public key format.");
        }

        std::string hexModulus;
        input >> hexModulus;

        PublicKey key;
        key.n = BigInt(hexModulus, 16);
        return key;
    }
};
