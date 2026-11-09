#include "ElGamal.hpp"

#include <algorithm>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>

ElGamal::ElGamal(const CurvePoint& publicKey) : publicKey_(publicKey)
{
}

ElGamal::ElGamal(const UInt256& privateKey)
    : privateKey_(privateKey), hasPrivateKey_(true)
{
}

bool ElGamal::encrypt(const CurvePoint& message, CipherBlock& ciphertext) const
{
    if (publicKey_.infinity || message.infinity) return false;
    const CurvePoint generator = EllipticCurve::generator();

    do {
        UInt256 nonce;
        if (!randomScalar(nonce)) return false;
        ciphertext.c1 = EllipticCurve::scalarMultiply(generator, nonce);
        const CurvePoint shared = EllipticCurve::scalarMultiply(publicKey_, nonce);
        ciphertext.c2 = EllipticCurve::pointAdd(message, shared);
        // The container stores finite points only; retry the rare cancellation.
    } while (ciphertext.c2.infinity);
    return true;
}

bool ElGamal::decrypt(const CipherBlock& ciphertext, CurvePoint& message) const
{
    if (!hasPrivateKey_ || privateKey_.zero() ||
        UInt256::compare(privateKey_, EllipticCurve::order()) >= 0 ||
        ciphertext.c1.infinity || ciphertext.c2.infinity ||
        !EllipticCurve::scalarMultiply(ciphertext.c1, EllipticCurve::order()).infinity) {
        return false;
    }
    const CurvePoint shared = EllipticCurve::scalarMultiply(ciphertext.c1, privateKey_);
    message = EllipticCurve::pointAdd(ciphertext.c2, EllipticCurve::negatePoint(shared));
    return !message.infinity;
}

bool ElGamal::randomScalar(UInt256& result)
{
    std::array<Byte, 32> bytes{};
    do {
        if (BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()),
                            BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
            return false;
        }
        result = UInt256::fromBytes(bytes);
    } while (result.zero() || UInt256::compare(result, EllipticCurve::order()) >= 0);
    return true;
}

bool ElGamal::generateKeyPair(UInt256& privateKey, CurvePoint& publicKey)
{
    if (!randomScalar(privateKey)) return false;
    publicKey = EllipticCurve::scalarMultiply(EllipticCurve::generator(), privateKey);
    return !publicKey.infinity;
}

UInt256 ElGamal::shiftLeft8(const UInt256& value)
{
    UInt256 result;
    for (unsigned i = 0; i < 4; ++i) {
        result.limb[i] = value.limb[i] << 8;
        if (i != 0) result.limb[i] |= value.limb[i - 1] >> 56;
    }
    return result;
}

UInt256 ElGamal::shiftRight8(const UInt256& value)
{
    UInt256 result;
    for (unsigned i = 0; i < 4; ++i) {
        result.limb[i] = value.limb[i] >> 8;
        if (i != 3) result.limb[i] |= value.limb[i + 1] << 56;
    }
    return result;
}

UInt256 ElGamal::bytesToInteger(const Byte* data, std::size_t size)
{
    UInt256 result;
    for (std::size_t i = 0; i < size; ++i) {
        result = shiftLeft8(result);
        result.limb[0] |= data[i];
    }
    return result;
}

bool ElGamal::messagePoint(const Byte* data, std::size_t size, CurvePoint& point)
{
    if (size == 0 || size > BlockSize || data == nullptr) return false;
    // Reversible encoding: x = 256*m + tag, 0 <= tag < 256.
    const UInt256 message = bytesToInteger(data, size);
    UInt256 x = shiftLeft8(message);

    for (unsigned tag = 0; tag < 256; ++tag) {
        x.limb[0] = (x.limb[0] & ~static_cast<std::uint64_t>(0xff)) | tag;
        if (UInt256::compare(x, EllipticCurve::field()) >= 0) continue;

        const UInt256 x2 = UInt256::mulMod(x, x, EllipticCurve::field());
        const UInt256 x3 = UInt256::mulMod(x2, x, EllipticCurve::field());
        const UInt256 rhs = UInt256::addMod(
            UInt256::addMod(x3, UInt256::mulMod(EllipticCurve::curveA(), x, EllipticCurve::field()), EllipticCurve::field()),
            EllipticCurve::curveB(), EllipticCurve::field());
        const UInt256 y = UInt256::powMod(rhs, EllipticCurve::squareRootExponent(), EllipticCurve::field());
        if (UInt256::compare(UInt256::mulMod(y, y, EllipticCurve::field()), rhs) == 0) {
            point = EllipticCurve::affinePoint(x, y);
            return true;
        }
    }
    return false;
}

bool ElGamal::pointMessage(const CurvePoint& point, std::array<Byte, ElGamal::BlockSize>& result)
{
    UInt256 x;
    UInt256 y;
    if (!EllipticCurve::toAffine(point, x, y)) return false;
    const UInt256 message = shiftRight8(x);

    // After removing the tag, m must fit in 30 bytes (240 bits).
    if ((message.limb[3] >> 48) != 0) return false;

    const auto bytes = message.toBytes();
    std::copy(bytes.end() - static_cast<std::ptrdiff_t>(result.size()), bytes.end(), result.begin());
    return true;
}
