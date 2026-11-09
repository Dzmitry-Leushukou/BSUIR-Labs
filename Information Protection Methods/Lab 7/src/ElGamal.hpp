#pragma once

#include "EllipticCurve.hpp"

struct CipherBlock {
    CurvePoint c1;
    CurvePoint c2;
};

class ElGamal {
public:
    static constexpr std::size_t BlockSize = 30;

    explicit ElGamal(const CurvePoint& publicKey);
    explicit ElGamal(const UInt256& privateKey);

    bool encrypt(const CurvePoint& message, CipherBlock& ciphertext) const;
    bool decrypt(const CipherBlock& ciphertext, CurvePoint& message) const;
    static bool messagePoint(const Byte* data, std::size_t size, CurvePoint& point);
    static bool pointMessage(const CurvePoint& point, std::array<Byte, BlockSize>& result);
    static bool generateKeyPair(UInt256& privateKey, CurvePoint& publicKey);

private:
    static UInt256 shiftLeft8(const UInt256& value);
    static UInt256 shiftRight8(const UInt256& value);
    static UInt256 bytesToInteger(const Byte* data, std::size_t size);
    static bool randomScalar(UInt256& result);
    CurvePoint publicKey_;
    UInt256 privateKey_;
    bool hasPrivateKey_ = false;
};
