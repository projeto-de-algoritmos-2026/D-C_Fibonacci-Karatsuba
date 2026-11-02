#pragma once

#include "BigInt.hpp"

namespace bignum {

class Multiplier {
public:
    virtual ~Multiplier() = default;

    [[nodiscard]] virtual BigInt multiply(const BigInt& lhs, const BigInt& rhs) const = 0;

protected:
    Multiplier() = default;
    Multiplier(const Multiplier&) = default;
    Multiplier& operator=(const Multiplier&) = default;
    Multiplier(Multiplier&&) = default;
    Multiplier& operator=(Multiplier&&) = default;
};

}
