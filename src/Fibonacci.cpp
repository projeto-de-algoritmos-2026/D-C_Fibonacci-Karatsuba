#include "Fibonacci.hpp"

#include <utility>

namespace bignum {

namespace {

constexpr int INDEX_BITS = 64;

}

Fibonacci::Fibonacci(const Multiplier& multiplier) : m_multiplier(&multiplier) {
}

BigInt Fibonacci::compute(std::uint64_t index) const {
    BigInt current("0");
    BigInt next("1");

    for (int bit = INDEX_BITS - 1; bit >= 0; --bit) {
        BigInt evenTerm = m_multiplier->multiply(current, next + next - current);
        BigInt oddTerm =
            m_multiplier->multiply(current, current) + m_multiplier->multiply(next, next);

        if (((index >> bit) & 1U) != 0) {
            next = evenTerm + oddTerm;
            current = std::move(oddTerm);
        } else {
            current = std::move(evenTerm);
            next = std::move(oddTerm);
        }
    }

    return current;
}

BigInt Fibonacci::computeIterative(std::uint64_t index) {
    BigInt current("0");
    BigInt next("1");

    for (std::uint64_t step = 0; step < index; ++step) {
        BigInt sum = current + next;
        current = std::move(next);
        next = std::move(sum);
    }

    return current;
}

}
