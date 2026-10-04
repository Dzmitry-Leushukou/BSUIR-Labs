#pragma once

#include <vector>
#include <string>
#include <cstdint>

class BigInt {
private:
    std::vector<int> digits_;
    bool negative_;

    static constexpr int BASE = 1000000000;
    static constexpr int BASE_DIGITS = 9;

public:
    BigInt();
    BigInt(int value);
    BigInt(const std::string& str, int base = 10);

    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;

    BigInt operator<<(int shift) const;
    BigInt operator>>(int shift) const;
    BigInt operator&(const BigInt& other) const;
    BigInt operator|(const BigInt& other) const;

    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
    bool operator<(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;

    BigInt& operator+=(const BigInt& other);
    BigInt& operator-=(const BigInt& other);

    void setBit(size_t pos);
    bool getBit(size_t pos) const;
    size_t bitLength() const;

    std::string toString(int base = 10) const;

    bool isZero() const;

private:
    void removeLeadingZeros();
    int compareAbs(const BigInt& other) const;
    BigInt addAbs(const BigInt& other) const;
    BigInt subAbs(const BigInt& other) const;
    void divMod(const BigInt& divisor, BigInt& quotient, BigInt& remainder) const;
};
