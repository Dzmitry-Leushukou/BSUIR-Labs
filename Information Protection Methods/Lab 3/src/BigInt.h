#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <iostream>

class BigInt {
public:
    BigInt() : digits_{0}, negative_(false) {}

    BigInt(int64_t value) : negative_(value < 0) {
        uint64_t abs_value = negative_ ? -static_cast<uint64_t>(value) : value;
        if (abs_value == 0) {
            digits_ = {0};
        } else {
            while (abs_value > 0) {
                digits_.push_back(abs_value % BASE);
                abs_value /= BASE;
            }
        }
    }

    BigInt(const std::string& str, int base = 10);

    bool isZero() const {
        return digits_.size() == 1 && digits_[0] == 0;
    }

    bool isNegative() const {
        return negative_ && !isZero();
    }

    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;

    BigInt& operator+=(const BigInt& other);
    BigInt& operator-=(const BigInt& other);
    BigInt& operator*=(const BigInt& other);
    BigInt& operator/=(const BigInt& other);
    BigInt& operator%=(const BigInt& other);

    BigInt& operator<<=(int shift);
    BigInt& operator>>=(int shift);
    BigInt operator<<(int shift) const;
    BigInt operator>>(int shift) const;

    BigInt& operator|=(const BigInt& other);
    BigInt& operator&=(const BigInt& other);
    BigInt operator|(const BigInt& other) const;
    BigInt operator&(const BigInt& other) const;

    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
    bool operator<(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;

    std::string toString(int base = 10) const;
    size_t bitLength() const;
    void setBit(size_t pos);
    bool getBit(size_t pos) const;

    static BigInt fromBytes(const std::vector<uint8_t>& bytes);
    std::vector<uint8_t> toBytes(size_t size) const;

private:
    static constexpr uint32_t BASE = 1000000000;
    static constexpr int BASE_DIGITS = 9;

    std::vector<uint32_t> digits_;
    bool negative_;

    void removeLeadingZeros();
    int compareAbs(const BigInt& other) const;
    BigInt addAbs(const BigInt& other) const;
    BigInt subAbs(const BigInt& other) const;
    void divMod(const BigInt& divisor, BigInt& quotient, BigInt& remainder) const;
};

BigInt::BigInt(const std::string& str, int base) : negative_(false) {
    if (str.empty()) {
        digits_ = {0};
        return;
    }

    size_t start = 0;
    if (str[0] == '-') {
        negative_ = true;
        start = 1;
    }

    if (base == 16) {
        digits_ = {0};
        for (size_t i = start; i < str.length(); ++i) {
            char c = str[i];
            int digit;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            else continue;

            *this = *this * BigInt(16) + BigInt(digit);
        }
    } else {
        digits_ = {0};
        for (size_t i = start; i < str.length(); ++i) {
            if (str[i] >= '0' && str[i] <= '9') {
                *this = *this * BigInt(10) + BigInt(str[i] - '0');
            }
        }
    }

    if (isZero()) negative_ = false;
}

void BigInt::removeLeadingZeros() {
    while (digits_.size() > 1 && digits_.back() == 0) {
        digits_.pop_back();
    }
    if (isZero()) negative_ = false;
}

int BigInt::compareAbs(const BigInt& other) const {
    if (digits_.size() != other.digits_.size()) {
        return digits_.size() < other.digits_.size() ? -1 : 1;
    }
    for (int i = static_cast<int>(digits_.size()) - 1; i >= 0; --i) {
        if (digits_[i] != other.digits_[i]) {
            return digits_[i] < other.digits_[i] ? -1 : 1;
        }
    }
    return 0;
}

BigInt BigInt::addAbs(const BigInt& other) const {
    BigInt result;
    result.digits_.clear();
    uint64_t carry = 0;
    size_t maxSize = std::max(digits_.size(), other.digits_.size());

    for (size_t i = 0; i < maxSize || carry; ++i) {
        uint64_t sum = carry;
        if (i < digits_.size()) sum += digits_[i];
        if (i < other.digits_.size()) sum += other.digits_[i];
        result.digits_.push_back(sum % BASE);
        carry = sum / BASE;
    }

    return result;
}

BigInt BigInt::subAbs(const BigInt& other) const {
    BigInt result;
    result.digits_.clear();
    int64_t borrow = 0;

    for (size_t i = 0; i < digits_.size(); ++i) {
        int64_t diff = digits_[i] - borrow;
        if (i < other.digits_.size()) diff -= other.digits_[i];

        if (diff < 0) {
            diff += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.digits_.push_back(diff);
    }

    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::operator+(const BigInt& other) const {
    if (negative_ == other.negative_) {
        BigInt result = addAbs(other);
        result.negative_ = negative_;
        return result;
    }

    int cmp = compareAbs(other);
    if (cmp == 0) return BigInt(0);

    if (cmp > 0) {
        BigInt result = subAbs(other);
        result.negative_ = negative_;
        return result;
    } else {
        BigInt result = other.subAbs(*this);
        result.negative_ = other.negative_;
        return result;
    }
}

BigInt BigInt::operator-(const BigInt& other) const {
    if (negative_ != other.negative_) {
        BigInt result = addAbs(other);
        result.negative_ = negative_;
        return result;
    }

    int cmp = compareAbs(other);
    if (cmp == 0) return BigInt(0);

    if (cmp > 0) {
        BigInt result = subAbs(other);
        result.negative_ = negative_;
        return result;
    } else {
        BigInt result = other.subAbs(*this);
        result.negative_ = !negative_;
        return result;
    }
}

BigInt BigInt::operator*(const BigInt& other) const {
    BigInt result;
    result.digits_.assign(digits_.size() + other.digits_.size(), 0);

    for (size_t i = 0; i < digits_.size(); ++i) {
        uint64_t carry = 0;
        for (size_t j = 0; j < other.digits_.size() || carry; ++j) {
            uint64_t current = result.digits_[i + j] +
                              digits_[i] * 1ULL * (j < other.digits_.size() ? other.digits_[j] : 0) + carry;
            result.digits_[i + j] = current % BASE;
            carry = current / BASE;
        }
    }

    result.removeLeadingZeros();
    result.negative_ = negative_ != other.negative_;
    if (result.isZero()) result.negative_ = false;
    return result;
}

void BigInt::divMod(const BigInt& divisor, BigInt& quotient, BigInt& remainder) const {
    if (divisor.isZero()) {
        throw std::domain_error("Division by zero");
    }

    quotient = BigInt(0);
    remainder = *this;
    remainder.negative_ = false;

    if (compareAbs(divisor) < 0) {
        quotient = BigInt(0);
        remainder = *this;
        remainder.negative_ = false;
        return;
    }

    BigInt current(0);
    quotient.digits_.clear();

    for (int i = static_cast<int>(digits_.size()) - 1; i >= 0; --i) {
        current = current * BigInt(BASE) + BigInt(static_cast<int64_t>(digits_[i]));

        int x = 0;
        int left = 0, right = BASE;
        while (left <= right) {
            int mid = (left + right) / 2;
            BigInt test = divisor * BigInt(mid);
            test.negative_ = false;
            if (test.compareAbs(current) <= 0) {
                x = mid;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }

        quotient.digits_.push_back(x);
        current = current - divisor * BigInt(x);
        current.negative_ = false;
    }

    std::reverse(quotient.digits_.begin(), quotient.digits_.end());
    quotient.removeLeadingZeros();
    remainder = current;
}

BigInt BigInt::operator/(const BigInt& other) const {
    BigInt quotient, remainder;
    divMod(other, quotient, remainder);
    quotient.negative_ = negative_ != other.negative_ && !quotient.isZero();
    return quotient;
}

BigInt BigInt::operator%(const BigInt& other) const {
    BigInt quotient, remainder;
    BigInt abs_this = *this;
    abs_this.negative_ = false;
    BigInt abs_other = other;
    abs_other.negative_ = false;

    abs_this.divMod(abs_other, quotient, remainder);

    if (negative_ && !remainder.isZero()) {
        remainder = abs_other - remainder;
    }

    return remainder;
}

BigInt& BigInt::operator+=(const BigInt& other) {
    *this = *this + other;
    return *this;
}

BigInt& BigInt::operator-=(const BigInt& other) {
    *this = *this - other;
    return *this;
}

BigInt& BigInt::operator*=(const BigInt& other) {
    *this = *this * other;
    return *this;
}

BigInt& BigInt::operator/=(const BigInt& other) {
    *this = *this / other;
    return *this;
}

BigInt& BigInt::operator%=(const BigInt& other) {
    *this = *this % other;
    return *this;
}

BigInt& BigInt::operator<<=(int shift) {
    *this = *this << shift;
    return *this;
}

BigInt& BigInt::operator>>=(int shift) {
    *this = *this >> shift;
    return *this;
}

BigInt BigInt::operator<<(int shift) const {
    if (shift == 0) return *this;
    if (isZero()) return *this;

    // Use power of 2 for shifting
    BigInt power(1);
    BigInt two(2);

    // Build 2^shift efficiently using binary exponentiation
    int s = shift;
    BigInt base(2);
    while (s > 0) {
        if ((s % 2) == 1) {
            power = power * base;
        }
        if (s > 1) {
            base = base * base;
        }
        s = s / 2;
    }

    BigInt result = *this * power;
    result.negative_ = negative_;
    return result;
}

BigInt BigInt::operator>>(int shift) const {
    if (shift == 0) return *this;
    if (isZero()) return *this;

    // Use power of 2 for shifting
    BigInt power(1);
    BigInt two(2);

    // Build 2^shift efficiently using binary exponentiation
    int s = shift;
    BigInt base(2);
    while (s > 0) {
        if ((s % 2) == 1) {
            power = power * base;
        }
        if (s > 1) {
            base = base * base;
        }
        s = s / 2;
    }

    BigInt result = *this / power;
    result.negative_ = negative_;
    return result;
}

BigInt& BigInt::operator|=(const BigInt& other) {
    *this = *this | other;
    return *this;
}

BigInt& BigInt::operator&=(const BigInt& other) {
    *this = *this & other;
    return *this;
}

BigInt BigInt::operator|(const BigInt& other) const {
    size_t maxBits = std::max(bitLength(), other.bitLength());
    BigInt result(0);

    for (size_t i = 0; i < maxBits; ++i) {
        if (getBit(i) || other.getBit(i)) {
            result.setBit(i);
        }
    }

    return result;
}

BigInt BigInt::operator&(const BigInt& other) const {
    size_t maxBits = std::max(bitLength(), other.bitLength());
    BigInt result(0);

    for (size_t i = 0; i < maxBits; ++i) {
        if (getBit(i) && other.getBit(i)) {
            result.setBit(i);
        }
    }

    return result;
}

bool BigInt::operator==(const BigInt& other) const {
    return negative_ == other.negative_ && digits_ == other.digits_;
}

bool BigInt::operator!=(const BigInt& other) const {
    return !(*this == other);
}

bool BigInt::operator<(const BigInt& other) const {
    if (negative_ != other.negative_) {
        return negative_;
    }

    int cmp = compareAbs(other);
    return negative_ ? cmp > 0 : cmp < 0;
}

bool BigInt::operator<=(const BigInt& other) const {
    return *this < other || *this == other;
}

bool BigInt::operator>(const BigInt& other) const {
    return !(*this <= other);
}

bool BigInt::operator>=(const BigInt& other) const {
    return !(*this < other);
}

std::string BigInt::toString(int base) const {
    if (isZero()) return "0";

    if (base == 16) {
        std::string result;
        BigInt temp = *this;
        temp.negative_ = false;

        while (!temp.isZero()) {
            int digit = (temp % BigInt(16)).digits_[0];
            result += (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            temp /= BigInt(16);
        }

        if (negative_) result += '-';
        std::reverse(result.begin(), result.end());
        return result;
    }

    std::string result;
    BigInt temp = *this;
    temp.negative_ = false;

    while (!temp.isZero()) {
        result += '0' + (temp % BigInt(10)).digits_[0];
        temp /= BigInt(10);
    }

    if (negative_) result += '-';
    std::reverse(result.begin(), result.end());
    return result;
}

size_t BigInt::bitLength() const {
    if (isZero()) return 1;

    BigInt temp = *this;
    temp.negative_ = false;
    size_t bits = 0;

    while (!temp.isZero()) {
        temp = temp >> 1;
        bits++;
    }

    return bits;
}

void BigInt::setBit(size_t pos) {
    // Direct implementation without using operator| or operator<<
    // Calculate which digit and which bit within that digit
    size_t bitsPerDigit = 30; // BASE = 10^9 ~ 2^30
    size_t digitIndex = pos / bitsPerDigit;
    size_t bitInDigit = pos % bitsPerDigit;

    // Expand digits array if needed
    while (digits_.size() <= digitIndex) {
        digits_.push_back(0);
    }

    // Set the bit using power of 2
    int64_t bitValue = 1LL << bitInDigit;
    digits_[digitIndex] |= bitValue;

    // Normalize if digit exceeds BASE
    if (digits_[digitIndex] >= BASE) {
        // Split the digit
        int64_t carry = digits_[digitIndex] / BASE;
        digits_[digitIndex] %= BASE;
        if (digitIndex + 1 < digits_.size()) {
            digits_[digitIndex + 1] += carry;
        } else {
            digits_.push_back(carry);
        }
    }
}

bool BigInt::getBit(size_t pos) const {
    // Direct implementation without using operator& or operator<<
    size_t bitsPerDigit = 30;
    size_t digitIndex = pos / bitsPerDigit;
    size_t bitInDigit = pos % bitsPerDigit;

    if (digitIndex >= digits_.size()) {
        return false;
    }

    int64_t bitValue = 1LL << bitInDigit;
    return (digits_[digitIndex] & bitValue) != 0;
}

BigInt BigInt::fromBytes(const std::vector<uint8_t>& bytes) {
    BigInt result(0);

    for (const uint8_t byte : bytes) {
        result = result << 8;
        result = result + BigInt(byte);
    }

    return result;
}

std::vector<uint8_t> BigInt::toBytes(size_t size) const {
    std::vector<uint8_t> bytes(size, 0);
    BigInt temp = *this;
    temp.negative_ = false;

    for (size_t i = size; i-- > 0;) {
        bytes[i] = (temp % BigInt(256)).digits_[0];
        temp = temp / BigInt(256);
    }

    return bytes;
}
