#include "EllipticCurve.hpp"

int UInt256::compare(const UInt256& left, const UInt256& right)
{
    for (int i = 3; i >= 0; --i) {
        if (left.limb[i] < right.limb[i]) return -1;
        if (left.limb[i] > right.limb[i]) return 1;
    }
    return 0;
}

UInt256 UInt256::addRaw(const UInt256& left, const UInt256& right, bool& carry)
{
    UInt256 result;
    carry = false;
    for (unsigned i = 0; i < 4; ++i) {
        const std::uint64_t first = left.limb[i] + right.limb[i];
        const bool firstCarry = first < left.limb[i];
        const std::uint64_t second = first + static_cast<std::uint64_t>(carry);
        const bool secondCarry = second < first;
        result.limb[i] = second;
        carry = firstCarry || secondCarry;
    }
    return result;
}

UInt256 UInt256::subRaw(const UInt256& left, const UInt256& right)
{
    UInt256 result;
    bool borrow = false;
    for (unsigned i = 0; i < 4; ++i) {
        const std::uint64_t first = left.limb[i] - right.limb[i];
        const bool firstBorrow = left.limb[i] < right.limb[i];
        const std::uint64_t second = first - static_cast<std::uint64_t>(borrow);
        const bool secondBorrow = first < static_cast<std::uint64_t>(borrow);
        result.limb[i] = second;
        borrow = firstBorrow || secondBorrow;
    }
    return result;
}

int UInt256::hexDigit(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

bool UInt256::fromHex(const std::string& text, UInt256& value)
{
    if (text.size() != 64) return false;
    value = {};
    for (std::size_t i = 0; i < text.size(); ++i) {
        const int nibble = hexDigit(text[text.size() - 1 - i]);
        if (nibble < 0) return false;
        value.limb[i / 16] |= static_cast<std::uint64_t>(nibble) << ((i % 16) * 4);
    }
    return true;
}

UInt256 UInt256::fromBytes(const std::array<Byte, 32>& bytes)
{
    UInt256 value;
    for (unsigned i = 0; i < bytes.size(); ++i) {
        value.limb[(31 - i) / 8] |=
            static_cast<std::uint64_t>(bytes[i]) << (((31 - i) % 8) * 8);
    }
    return value;
}

std::string UInt256::toHex(const UInt256& value)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string result(64, '0');
    for (unsigned i = 0; i < 32; ++i) {
        const Byte byte = static_cast<Byte>(value.limb[(31 - i) / 8] >> (((31 - i) % 8) * 8));
        result[i * 2] = digits[byte >> 4];
        result[i * 2 + 1] = digits[byte & 0x0f];
    }
    return result;
}

std::array<Byte, 32> UInt256::toBytes() const
{
    std::array<Byte, 32> result{};
    for (unsigned i = 0; i < result.size(); ++i) {
        result[i] = static_cast<Byte>(limb[(31 - i) / 8] >> (((31 - i) % 8) * 8));
    }
    return result;
}

UInt256 UInt256::addMod(const UInt256& left, const UInt256& right, const UInt256& modulus)
{
    bool carry = false;
    UInt256 sum = addRaw(left, right, carry);
    if (carry || compare(sum, modulus) >= 0) sum = subRaw(sum, modulus);
    return sum;
}

UInt256 UInt256::subMod(const UInt256& left, const UInt256& right, const UInt256& modulus)
{
    if (compare(left, right) >= 0) return subRaw(left, right);
    return subRaw(modulus, subRaw(right, left));
}

UInt256 UInt256::mulMod(const UInt256& left, const UInt256& right, const UInt256& modulus)
{
    UInt256 result;
    UInt256 addend = left;
    for (unsigned bit = 0; bit < 256; ++bit) {
        if (right.bit(bit)) result = addMod(result, addend, modulus);
        if (bit != 255) addend = addMod(addend, addend, modulus);
    }
    return result;
}

UInt256 UInt256::powMod(UInt256 base, const UInt256& exponent, const UInt256& modulus)
{
    UInt256 result;
    result.limb[0] = 1;
    for (unsigned bit = 0; bit < 256; ++bit) {
        if (exponent.bit(bit)) result = mulMod(result, base, modulus);
        if (bit != 255) base = mulMod(base, base, modulus);
    }
    return result;
}

UInt256 UInt256::subtractSmall(UInt256 value, std::uint64_t amount)
{
    UInt256 small;
    small.limb[0] = amount;
    return subRaw(value, small);
}

CurvePoint EllipticCurve::generator()
{
    return affinePoint(generatorX(), generatorY());
}

const UInt256& EllipticCurve::field()
{
    static const UInt256 value = [] {
        UInt256 result;
        UInt256::fromHex("fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd97", result);
        return result;
    }();
    return value;
}

const UInt256& EllipticCurve::order()
{
    static const UInt256 value = [] {
        UInt256 result;
        UInt256::fromHex("400000000000000000000000000000000fd8cddfc87b6635c115af556c360c67", result);
        return result;
    }();
    return value;
}

const UInt256& EllipticCurve::curveA()
{
    static const UInt256 value = [] {
        UInt256 result;
        UInt256::fromHex("c2173f1513981673af4892c23035a27ce25e2013bf95aa33b22c656f277e7335", result);
        return result;
    }();
    return value;
}

const UInt256& EllipticCurve::generatorX()
{
    static const UInt256 value = [] {
        UInt256 result;
        UInt256::fromHex("91e38443a5e82c0d880923425712b2bb658b9196932e02c78b2582fe742daa28", result);
        return result;
    }();
    return value;
}

const UInt256& EllipticCurve::generatorY()
{
    static const UInt256 value = [] {
        UInt256 result;
        UInt256::fromHex("32879423ab1a0375895786c4bb46e9565fde0b5344766740af268adb32322e5c", result);
        return result;
    }();
    return value;
}

CurvePoint EllipticCurve::pointAtInfinity()
{
    return {};
}

CurvePoint EllipticCurve::affinePoint(const UInt256& x, const UInt256& y)
{
    CurvePoint result;
    result.x = x;
    result.y = y;
    result.z.limb[0] = 1;
    result.infinity = false;
    return result;
}

CurvePoint EllipticCurve::pointDouble(const CurvePoint& point)
{
    if (point.infinity || point.y.zero()) return pointAtInfinity();

    const UInt256& p = field();
    const UInt256 xx = UInt256::mulMod(point.x, point.x, p);
    const UInt256 yy = UInt256::mulMod(point.y, point.y, p);
    const UInt256 yyyy = UInt256::mulMod(yy, yy, p);
    const UInt256 zz = UInt256::mulMod(point.z, point.z, p);
    const UInt256 s = UInt256::mulMod(
        UInt256{{4, 0, 0, 0}},
        UInt256::mulMod(point.x, yy, p), p);
    const UInt256 m = UInt256::addMod(
        UInt256::mulMod(UInt256{{3, 0, 0, 0}}, xx, p),
        UInt256::mulMod(curveA(), UInt256::mulMod(zz, zz, p), p), p);
    const UInt256 x3 = UInt256::subMod(UInt256::mulMod(m, m, p), UInt256::addMod(s, s, p), p);
    const UInt256 eightYyyy = UInt256::mulMod(UInt256{{8, 0, 0, 0}}, yyyy, p);
    const UInt256 y3 = UInt256::subMod(
        UInt256::mulMod(m, UInt256::subMod(s, x3, p), p), eightYyyy, p);
    const UInt256 yPlusZ = UInt256::addMod(point.y, point.z, p);
    const UInt256 z3 = UInt256::subMod(
        UInt256::subMod(UInt256::mulMod(yPlusZ, yPlusZ, p), yy, p), zz, p);

    return CurvePoint{x3, y3, z3, false};
}

CurvePoint EllipticCurve::pointAdd(const CurvePoint& left, const CurvePoint& right)
{
    if (left.infinity) return right;
    if (right.infinity) return left;
    const UInt256& p = field();

    const UInt256 z1z1 = UInt256::mulMod(left.z, left.z, p);
    const UInt256 z2z2 = UInt256::mulMod(right.z, right.z, p);
    const UInt256 u1 = UInt256::mulMod(left.x, z2z2, p);
    const UInt256 u2 = UInt256::mulMod(right.x, z1z1, p);
    const UInt256 s1 = UInt256::mulMod(left.y, UInt256::mulMod(right.z, z2z2, p), p);
    const UInt256 s2 = UInt256::mulMod(right.y, UInt256::mulMod(left.z, z1z1, p), p);

    if (UInt256::compare(u1, u2) == 0) {
        return UInt256::compare(s1, s2) == 0 ? pointDouble(left) : pointAtInfinity();
    }

    const UInt256 h = UInt256::subMod(u2, u1, p);
    const UInt256 i = UInt256::mulMod(UInt256{{4, 0, 0, 0}}, UInt256::mulMod(h, h, p), p);
    const UInt256 j = UInt256::mulMod(h, i, p);
    const UInt256 r = UInt256::mulMod(UInt256{{2, 0, 0, 0}}, UInt256::subMod(s2, s1, p), p);
    const UInt256 v = UInt256::mulMod(u1, i, p);
    const UInt256 x3 = UInt256::subMod(UInt256::subMod(UInt256::mulMod(r, r, p), j, p), UInt256::addMod(v, v, p), p);
    const UInt256 y3 = UInt256::subMod(
        UInt256::mulMod(r, UInt256::subMod(v, x3, p), p),
        UInt256::mulMod(UInt256{{2, 0, 0, 0}}, UInt256::mulMod(s1, j, p), p), p);
    const UInt256 zSum = UInt256::addMod(left.z, right.z, p);
    const UInt256 z3 = UInt256::mulMod(
        UInt256::subMod(UInt256::subMod(UInt256::mulMod(zSum, zSum, p), z1z1, p), z2z2, p), h, p);
    return CurvePoint{x3, y3, z3, false};
}

CurvePoint EllipticCurve::scalarMultiply(const CurvePoint& point, const UInt256& scalar)
{
    CurvePoint result = pointAtInfinity();
    CurvePoint addend = point;
    for (unsigned bit = 0; bit < 256; ++bit) {
        if (scalar.bit(bit)) result = pointAdd(result, addend);
        if (bit != 255) addend = pointDouble(addend);
    }
    return result;
}

bool EllipticCurve::toAffine(const CurvePoint& point, UInt256& x, UInt256& y)
{
    if (point.infinity) return false;
    const UInt256 zInverse = UInt256::powMod(point.z, UInt256::subtractSmall(field(), 2), field());
    const UInt256 zInverse2 = UInt256::mulMod(zInverse, zInverse, field());
    const UInt256 zInverse3 = UInt256::mulMod(zInverse2, zInverse, field());
    x = UInt256::mulMod(point.x, zInverse2, field());
    y = UInt256::mulMod(point.y, zInverse3, field());
    return true;
}

bool EllipticCurve::isOnCurve(const UInt256& x, const UInt256& y)
{
    if (UInt256::compare(x, field()) >= 0 || UInt256::compare(y, field()) >= 0) return false;
    const UInt256 left = UInt256::mulMod(y, y, field());
    const UInt256 x2 = UInt256::mulMod(x, x, field());
    const UInt256 x3 = UInt256::mulMod(x2, x, field());
    const UInt256 right = UInt256::addMod(
        UInt256::addMod(x3, UInt256::mulMod(curveA(), x, field()), field()),
        curveB(), field());
    return UInt256::compare(left, right) == 0;
}

const UInt256& EllipticCurve::curveB()
{
    static const UInt256 value = [] {
        UInt256 result;
        UInt256::fromHex("295f9bae7428ed9ccc20e7c359a9d41a22fccd9108e17bf7ba9337a6f8ae9513", result);
        return result;
    }();
    return value;
}

const UInt256& EllipticCurve::squareRootExponent()
{
    static const UInt256 value = [] {
        UInt256 result;
        UInt256::fromHex("3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff66", result);
        return result;
    }();
    return value;
}

CurvePoint EllipticCurve::negatePoint(CurvePoint point)
{
    if (!point.infinity && !point.y.zero()) point.y = UInt256::subRaw(field(), point.y);
    return point;
}

bool EllipticCurve::validPublicKey(const UInt256& x, const UInt256& y)
{
    if (!isOnCurve(x, y)) return false;
    const CurvePoint key = affinePoint(x, y);
    return scalarMultiply(key, order()).infinity;
}
