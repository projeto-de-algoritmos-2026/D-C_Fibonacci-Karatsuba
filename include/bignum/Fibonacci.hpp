#pragma once

#include <cstdint>

#include "BigInt.hpp"
#include "Multiplier.hpp"

namespace bignum {

class Fibonacci {
private:
    const Multiplier* m_multiplier;

public:
    explicit Fibonacci(const Multiplier& multiplier);

    [[nodiscard]] BigInt compute(std::uint64_t index) const;
    [[nodiscard]] static BigInt computeIterative(std::uint64_t index);
};

}
