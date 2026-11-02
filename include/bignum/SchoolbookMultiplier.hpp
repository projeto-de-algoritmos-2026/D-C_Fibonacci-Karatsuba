#pragma once

#include "BigInt.hpp"
#include "Multiplier.hpp"

namespace bignum {

class SchoolbookMultiplier final : public Multiplier {
public:
    [[nodiscard]] BigInt multiply(const BigInt& lhs, const BigInt& rhs) const override;

    [[nodiscard]] static Limbs multiplyLimbs(const Limbs& lhs, const Limbs& rhs);
};

}
