#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

using Byte = std::uint8_t;
using Bytes = std::vector<Byte>;

struct UInt256 {
    std::array<std::uint64_t, 4> limb{};

    bool zero() const
    {
        return limb[0] == 0 && limb[1] == 0 && limb[2] == 0 && limb[3] == 0;
    }

    bool bit(unsigned index) const
    {
        return ((limb[index / 64] >> (index % 64)) & 1U) != 0;
    }

    static int compare(const UInt256& left, const UInt256& right);
    static UInt256 addRaw(const UInt256& left, const UInt256& right, bool& carry);
    static UInt256 subRaw(const UInt256& left, const UInt256& right);
    static int hexDigit(char value);
    static bool fromHex(const std::string& text, UInt256& value);
    static UInt256 fromBytes(const std::array<Byte, 32>& bytes);
    std::array<Byte, 32> toBytes() const;
    static std::string toHex(const UInt256& value);
    static UInt256 addMod(const UInt256& left, const UInt256& right, const UInt256& modulus);
    static UInt256 subMod(const UInt256& left, const UInt256& right, const UInt256& modulus);
    static UInt256 mulMod(const UInt256& left, const UInt256& right, const UInt256& modulus);
    static UInt256 powMod(UInt256 base, const UInt256& exponent, const UInt256& modulus);
    static UInt256 subtractSmall(UInt256 value, std::uint64_t amount);
};

// Jacobian coordinates: affine x = X/Z^2, affine y = Y/Z^3.
struct CurvePoint {
    UInt256 x;
    UInt256 y;
    UInt256 z;
    bool infinity = true;
};

class EllipticCurve {
public:
    static const UInt256& field();
    static const UInt256& order();
    static const UInt256& curveA();
    static const UInt256& generatorX();
    static const UInt256& generatorY();
    static CurvePoint pointAtInfinity();
    static CurvePoint affinePoint(const UInt256& x, const UInt256& y);
    static CurvePoint pointDouble(const CurvePoint& point);
    static CurvePoint pointAdd(const CurvePoint& left, const CurvePoint& right);
    static CurvePoint scalarMultiply(const CurvePoint& point, const UInt256& scalar);
    static bool toAffine(const CurvePoint& point, UInt256& x, UInt256& y);
    static bool isOnCurve(const UInt256& x, const UInt256& y);
    static const UInt256& curveB();
    static const UInt256& squareRootExponent();
    static CurvePoint negatePoint(CurvePoint point);
    static bool validPublicKey(const UInt256& x, const UInt256& y);
    static CurvePoint generator();
};
