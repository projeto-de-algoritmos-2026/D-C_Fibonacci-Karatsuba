#pragma once

#include <cstddef>

#include "BigInt.hpp"
#include "Multiplier.hpp"

namespace bignum {

class KaratsubaMultiplier final : public Multiplier {
private:
    std::size_t m_threshold;

    [[nodiscard]] Limbs multiplyLimbs(const Limbs& lhs, const Limbs& rhs) const;

public:
    static constexpr std::size_t DEFAULT_THRESHOLD = 32;

    explicit KaratsubaMultiplier(std::size_t threshold = DEFAULT_THRESHOLD);

    [[nodiscard]] BigInt multiply(const BigInt& lhs, const BigInt& rhs) const override;
    [[nodiscard]] std::size_t threshold() const noexcept;
};

}
