#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bignum {

using Limb = std::uint32_t;
using DoubleLimb = std::uint64_t;
using Limbs = std::vector<Limb>;

constexpr int LIMB_BITS = 32;

class BigInt {
private:
    static constexpr std::size_t DECIMAL_CHUNK_DIGITS = 9;
    static constexpr Limb DECIMAL_CHUNK = 1'000'000'000;

    Limbs m_limbs;

    explicit BigInt(Limbs limbs);

    void multiplySmallAndAdd(Limb factor, Limb addend);
    static Limb divideSmallInPlace(Limbs& limbs, Limb divisor);
    static bool isZero(const Limbs& limbs);
    static void normalize(Limbs& limbs);
    static int compare(const Limbs& lhs, const Limbs& rhs);
    static void subtractInPlace(Limbs& minuend, const Limbs& subtrahend);
    static void shiftLeftOneBitInPlace(Limbs& limbs, Limb incomingBit);
    static void divideWithRemainder(const Limbs& dividend, const Limbs& divisor, Limbs& quotient,
                                    Limbs& remainder);

public:
    explicit BigInt(const std::string& value);

    [[nodiscard]] static BigInt fromLimbs(Limbs limbs);
    [[nodiscard]] const Limbs& limbs() const noexcept;
    [[nodiscard]] std::string toString() const;

    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
    bool operator<(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;

    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;
};

}
