#include "SchoolbookMultiplier.hpp"

namespace bignum {

BigInt SchoolbookMultiplier::multiply(const BigInt& lhs, const BigInt& rhs) const {
    return BigInt::fromLimbs(multiplyLimbs(lhs.limbs(), rhs.limbs()));
}

Limbs SchoolbookMultiplier::multiplyLimbs(const Limbs& lhs, const Limbs& rhs) {
    Limbs result(lhs.size() + rhs.size(), 0);

    for (std::size_t row = 0; row < lhs.size(); ++row) {
        DoubleLimb carry = 0;

        for (std::size_t column = 0; column < rhs.size(); ++column) {
            const DoubleLimb total = static_cast<DoubleLimb>(result[row + column]) +
                                     static_cast<DoubleLimb>(lhs[row]) * rhs[column] + carry;
            result[row + column] = static_cast<Limb>(total);
            carry = total >> LIMB_BITS;
        }

        result[row + rhs.size()] = static_cast<Limb>(carry);
    }

    return result;
}

}
