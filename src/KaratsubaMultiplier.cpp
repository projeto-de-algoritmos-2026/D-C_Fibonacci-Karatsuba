#include "KaratsubaMultiplier.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>

#include "SchoolbookMultiplier.hpp"

namespace bignum {

namespace {

void trimLeadingZeros(Limbs& limbs) {
    while (!limbs.empty() && limbs.back() == 0) {
        limbs.pop_back();
    }
}

Limbs sliceLimbs(const Limbs& source, std::size_t begin, std::size_t end) {
    const auto first = static_cast<std::ptrdiff_t>(std::min(begin, source.size()));
    const auto last = static_cast<std::ptrdiff_t>(std::min(end, source.size()));

    Limbs part(std::next(source.begin(), first), std::next(source.begin(), last));
    trimLeadingZeros(part);
    return part;
}

Limbs addLimbs(const Limbs& lhs, const Limbs& rhs) {
    const Limbs& longer = lhs.size() >= rhs.size() ? lhs : rhs;
    const Limbs& shorter = lhs.size() >= rhs.size() ? rhs : lhs;

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

    return result;
}

void subtractLimbsInPlace(Limbs& minuend, const Limbs& subtrahend) {
    DoubleLimb borrow = 0;

    for (std::size_t index = 0; index < minuend.size(); ++index) {
        if (index >= subtrahend.size() && borrow == 0) {
            break;
        }

        const DoubleLimb toSubtract = (index < subtrahend.size() ? subtrahend[index] : 0) + borrow;
        const DoubleLimb current = minuend[index];

        minuend[index] = static_cast<Limb>(current - toSubtract);
        borrow = current < toSubtract ? 1 : 0;
    }

    trimLeadingZeros(minuend);
}

void addShiftedInPlace(Limbs& result, const Limbs& value, std::size_t shift) {
    DoubleLimb carry = 0;
    std::size_t index = 0;

    for (; index < value.size(); ++index) {
        const DoubleLimb sum =
            static_cast<DoubleLimb>(result[shift + index]) + value[index] + carry;
        result[shift + index] = static_cast<Limb>(sum);
        carry = sum >> LIMB_BITS;
    }

    for (std::size_t position = shift + index; carry != 0; ++position) {
        const DoubleLimb sum = static_cast<DoubleLimb>(result[position]) + carry;
        result[position] = static_cast<Limb>(sum);
        carry = sum >> LIMB_BITS;
    }
}

}

KaratsubaMultiplier::KaratsubaMultiplier(std::size_t threshold) : m_threshold(threshold) {
    if (m_threshold == 0) {
        throw std::invalid_argument("KaratsubaMultiplier: threshold deve ser >= 1");
    }
}

std::size_t KaratsubaMultiplier::threshold() const noexcept {
    return m_threshold;
}

BigInt KaratsubaMultiplier::multiply(const BigInt& lhs, const BigInt& rhs) const {
    return BigInt::fromLimbs(multiplyLimbs(lhs.limbs(), rhs.limbs()));
}

Limbs KaratsubaMultiplier::multiplyLimbs(const Limbs& lhs, const Limbs& rhs) const {
    if (std::min(lhs.size(), rhs.size()) <= m_threshold) {
        Limbs product = SchoolbookMultiplier::multiplyLimbs(lhs, rhs);
        trimLeadingZeros(product);
        return product;
    }

    const std::size_t half = std::max(lhs.size(), rhs.size()) / 2;

    const Limbs lhsLow = sliceLimbs(lhs, 0, half);
    const Limbs lhsHigh = sliceLimbs(lhs, half, lhs.size());
    const Limbs rhsLow = sliceLimbs(rhs, 0, half);
    const Limbs rhsHigh = sliceLimbs(rhs, half, rhs.size());

    const Limbs lowProduct = multiplyLimbs(lhsLow, rhsLow);
    const Limbs highProduct = multiplyLimbs(lhsHigh, rhsHigh);
    Limbs middleProduct = multiplyLimbs(addLimbs(lhsLow, lhsHigh), addLimbs(rhsLow, rhsHigh));
    subtractLimbsInPlace(middleProduct, lowProduct);
    subtractLimbsInPlace(middleProduct, highProduct);

    Limbs product(lhs.size() + rhs.size() + 1, 0);
    addShiftedInPlace(product, lowProduct, 0);
    addShiftedInPlace(product, middleProduct, half);
    addShiftedInPlace(product, highProduct, 2 * half);

    trimLeadingZeros(product);
    return product;
}

}
