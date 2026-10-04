#pragma once

#include "BigInt.h"
#include <string>
#include <fstream>
#include <stdexcept>
#include <utility>

class PrivateKey {
public:
    BigInt p;
    BigInt q;
    BigInt n;

    PrivateKey() = default;

    PrivateKey(BigInt p_, BigInt q_)
        : p(std::move(p_)), q(std::move(q_)), n(p * q) {}

    void save(const std::string& path) const {
        std::ofstream output(path);

        if (!output) {
            throw std::runtime_error("Cannot create private key file.");
        }

        output << "RABIN_PRIVATE_V1\n";
        output << p.toString(16) << '\n';
        output << q.toString(16) << '\n';
    }

    static PrivateKey load(const std::string& path) {
        std::ifstream input(path);

        if (!input) {
            throw std::runtime_error("Cannot open private key file.");
        }

        std::string header;
        input >> header;

        if (header != "RABIN_PRIVATE_V1") {
            throw std::runtime_error("Invalid private key format.");
        }

        std::string pHex, qHex;
        input >> pHex >> qHex;

        return PrivateKey(BigInt(pHex, 16), BigInt(qHex, 16));
    }
};
