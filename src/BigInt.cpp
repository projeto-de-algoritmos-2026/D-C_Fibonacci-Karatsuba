#include "BigInt.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace bignum {

namespace {

constexpr Limb DECIMAL_BASE = 10;
constexpr DoubleLimb LIMB_BASE = DoubleLimb{1} << LIMB_BITS;

}

BigInt::BigInt(Limbs limbs) : m_limbs(std::move(limbs)) {
    if (m_limbs.empty()) {
        m_limbs.push_back(0);
    }
    normalize(m_limbs);
}

BigInt::BigInt(const std::string& value) : m_limbs{0} {
    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("BigInt: valor deve conter apenas digitos: \"" + value + "\"");
    }

    const auto firstNonZero = value.find_first_not_of('0');
    if (firstNonZero == std::string::npos) {
        return;
    }

    const std::string digits = value.substr(firstNonZero);
    std::size_t chunkLength = digits.size() % DECIMAL_CHUNK_DIGITS;
    if (chunkLength == 0) {
        chunkLength = DECIMAL_CHUNK_DIGITS;
    }

    for (std::size_t position = 0; position < digits.size();
         position += chunkLength, chunkLength = DECIMAL_CHUNK_DIGITS) {
        const auto chunk = static_cast<Limb>(std::stoul(digits.substr(position, chunkLength)));

        Limb scale = 1;
        for (std::size_t digit = 0; digit < chunkLength; ++digit) {
            scale *= DECIMAL_BASE;
        }

        multiplySmallAndAdd(scale, chunk);
    }
}

BigInt BigInt::fromLimbs(Limbs limbs) {
    return BigInt(std::move(limbs));
}

const Limbs& BigInt::limbs() const noexcept {
    return m_limbs;
}

std::string BigInt::toString() const {
    Limbs remaining = m_limbs;
    std::vector<Limb> chunks{divideSmallInPlace(remaining, DECIMAL_CHUNK)};

    while (!isZero(remaining)) {
        chunks.push_back(divideSmallInPlace(remaining, DECIMAL_CHUNK));
    }

    std::string result = std::to_string(chunks.back());

    for (auto chunk = chunks.rbegin() + 1; chunk != chunks.rend(); ++chunk) {
        const std::string part = std::to_string(*chunk);
        result += std::string(DECIMAL_CHUNK_DIGITS - part.size(), '0') + part;
    }

    return result;
}

void BigInt::multiplySmallAndAdd(Limb factor, Limb addend) {
    DoubleLimb carry = addend;

    for (Limb& limb : m_limbs) {
        const DoubleLimb current = static_cast<DoubleLimb>(limb) * factor + carry;
        limb = static_cast<Limb>(current);
        carry = current >> LIMB_BITS;
    }

    if (carry != 0) {
        m_limbs.push_back(static_cast<Limb>(carry));
    }
}

Limb BigInt::divideSmallInPlace(Limbs& limbs, Limb divisor) {
    DoubleLimb remainder = 0;

    for (std::size_t index = limbs.size(); index-- > 0;) {
        const DoubleLimb current = (remainder << LIMB_BITS) | limbs[index];
        limbs[index] = static_cast<Limb>(current / divisor);
        remainder = current % divisor;
    }

    normalize(limbs);
    return static_cast<Limb>(remainder);
}

bool BigInt::isZero(const Limbs& limbs) {
    return limbs.size() == 1 && limbs[0] == 0;
}

void BigInt::normalize(Limbs& limbs) {
    while (limbs.size() > 1 && limbs.back() == 0) {
        limbs.pop_back();
    }
}

int BigInt::compare(const Limbs& lhs, const Limbs& rhs) {
    if (lhs.size() != rhs.size()) {
        return lhs.size() < rhs.size() ? -1 : 1;
    }

    for (std::size_t index = lhs.size(); index-- > 0;) {
        if (lhs[index] != rhs[index]) {
            return lhs[index] < rhs[index] ? -1 : 1;
        }
    }

    return 0;
}

void BigInt::subtractInPlace(Limbs& minuend, const Limbs& subtrahend) {
    DoubleLimb borrow = 0;

    for (std::size_t index = 0; index < minuend.size(); ++index) {
        if (index >= subtrahend.size() && borrow == 0) {
            break;
        }

        const DoubleLimb toSubtract = borrow + (index < subtrahend.size() ? subtrahend[index] : 0);
        const DoubleLimb current = minuend[index];

        if (current < toSubtract) {
            minuend[index] = static_cast<Limb>(current + LIMB_BASE - toSubtract);
            borrow = 1;
        } else {
            minuend[index] = static_cast<Limb>(current - toSubtract);
            borrow = 0;
        }
    }

    normalize(minuend);
}

void BigInt::shiftLeftOneBitInPlace(Limbs& limbs, Limb incomingBit) {
    Limb carry = incomingBit;

    for (Limb& limb : limbs) {
        const Limb outgoingBit = limb >> (LIMB_BITS - 1);
        limb = (limb << 1) | carry;
        carry = outgoingBit;
    }

    if (carry != 0) {
        limbs.push_back(carry);
    }
}

void BigInt::divideWithRemainder(const Limbs& dividend, const Limbs& divisor, Limbs& quotient,
                                 Limbs& remainder) {
    if (isZero(divisor)) {
        throw std::domain_error("BigInt: divisao por zero");
    }

    if (compare(dividend, divisor) < 0) {
        quotient = {0};
        remainder = dividend;
        return;
    }

    if (divisor.size() == 1) {
        quotient = dividend;
        remainder = {divideSmallInPlace(quotient, divisor[0])};
        return;
    }

    quotient.assign(dividend.size(), 0);
    remainder = {0};

    for (std::size_t bitIndex = dividend.size() * LIMB_BITS; bitIndex-- > 0;) {
        const std::size_t limbIndex = bitIndex / LIMB_BITS;
        const auto bitPosition = static_cast<int>(bitIndex % LIMB_BITS);
        const Limb bit = (dividend[limbIndex] >> bitPosition) & 1U;

        shiftLeftOneBitInPlace(remainder, bit);

        if (compare(remainder, divisor) >= 0) {
            subtractInPlace(remainder, divisor);
            quotient[limbIndex] |= (Limb{1} << bitPosition);
        }
    }

    normalize(quotient);
}

bool BigInt::operator==(const BigInt& other) const {
    return m_limbs == other.m_limbs;
}

bool BigInt::operator!=(const BigInt& other) const {
    return !(*this == other);
}

bool BigInt::operator<(const BigInt& other) const {
    return compare(m_limbs, other.m_limbs) < 0;
}

bool BigInt::operator>(const BigInt& other) const {
    return other < *this;
}

bool BigInt::operator<=(const BigInt& other) const {
    return !(other < *this);
}

bool BigInt::operator>=(const BigInt& other) const {
    return !(*this < other);
}

BigInt BigInt::operator+(const BigInt& other) const {
    const Limbs& longer = m_limbs.size() >= other.m_limbs.size() ? m_limbs : other.m_limbs;
    const Limbs& shorter = m_limbs.size() >= other.m_limbs.size() ? other.m_limbs : m_limbs;

    Limbs result;
    result.reserve(longer.size() + 1);
    DoubleLimb carry = 0;

    for (std::size_t index = 0; index < longer.size(); ++index) {
        const DoubleLimb sum = static_cast<DoubleLimb>(longer[index]) +
                               (index < shorter.size() ? shorter[index] : 0) + carry;
        result.push_back(static_cast<Limb>(sum));
        carry = sum >> LIMB_BITS;
    }

    if (carry != 0) {
        result.push_back(static_cast<Limb>(carry));
    }

    return BigInt(std::move(result));
}

BigInt BigInt::operator-(const BigInt& other) const {
    if (*this < other) {
        throw std::underflow_error("BigInt: resultado negativo na subtracao");
    }

    Limbs result = m_limbs;
    subtractInPlace(result, other.m_limbs);
    return BigInt(std::move(result));
}

BigInt BigInt::operator/(const BigInt& other) const {
    Limbs quotient;
    Limbs remainder;
    divideWithRemainder(m_limbs, other.m_limbs, quotient, remainder);
    return BigInt(std::move(quotient));
}

BigInt BigInt::operator%(const BigInt& other) const {
    Limbs quotient;
    Limbs remainder;
    divideWithRemainder(m_limbs, other.m_limbs, quotient, remainder);
    return BigInt(std::move(remainder));
}

}
