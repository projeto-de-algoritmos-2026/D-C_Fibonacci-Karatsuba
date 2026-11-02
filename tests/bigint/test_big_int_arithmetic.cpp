#include <gtest/gtest.h>

#include <random>
#include <stdexcept>

#include "BigInt.hpp"
#include "support/RandomDigits.hpp"

namespace bignum {

TEST(BigIntAddition, AddsSmallNumbers) {
    EXPECT_EQ(BigInt("2") + BigInt("3"), BigInt("5"));
}

TEST(BigIntAddition, ZeroIsNeutral) {
    EXPECT_EQ(BigInt("12345") + BigInt("0"), BigInt("12345"));
    EXPECT_EQ(BigInt("0") + BigInt("12345"), BigInt("12345"));
}

TEST(BigIntAddition, CarriesIntoNewLimb) {
    EXPECT_EQ(BigInt("4294967295") + BigInt("1"), BigInt("4294967296"));
    EXPECT_EQ(BigInt("18446744073709551615") + BigInt("1"), BigInt("18446744073709551616"));
}

TEST(BigIntAddition, CrossesDecimalChunkBoundary) {
    EXPECT_EQ(BigInt("999999999") + BigInt("1"), BigInt("1000000000"));
}

TEST(BigIntSubtraction, SubtractsSmallNumbers) {
    EXPECT_EQ(BigInt("10") - BigInt("3"), BigInt("7"));
}

TEST(BigIntSubtraction, SameValueGivesZero) {
    EXPECT_EQ(BigInt("123456789012345678901234567890") - BigInt("123456789012345678901234567890"),
              BigInt("0"));
}

TEST(BigIntSubtraction, BorrowsAcrossLimbs) {
    EXPECT_EQ(BigInt("18446744073709551616") - BigInt("1"), BigInt("18446744073709551615"));
    EXPECT_EQ(BigInt("100000000000000000000") - BigInt("1"), BigInt("99999999999999999999"));
}

TEST(BigIntSubtraction, ThrowsWhenResultWouldBeNegative) {
    EXPECT_THROW((void)(BigInt("3") - BigInt("10")), std::underflow_error);
}

TEST(BigIntDivision, DividesBySingleLimb) {
    EXPECT_EQ(BigInt("123456789012345678901234567890") / BigInt("987654321"),
              BigInt("124999998873437499901"));
    EXPECT_EQ(BigInt("123456789012345678901234567890") % BigInt("987654321"), BigInt("574845669"));
}

TEST(BigIntDivision, DividesByMultipleLimbs) {
    const BigInt dividend("340282366920938463463374607431768223801");
    const BigInt divisor("18446744073709551623");

    EXPECT_EQ(dividend / divisor, BigInt("18446744073709551609"));
    EXPECT_EQ(dividend % divisor, BigInt("12394"));
}

TEST(BigIntDivision, SmallerDividendGivesZeroQuotient) {
    EXPECT_EQ(BigInt("5") / BigInt("7"), BigInt("0"));
    EXPECT_EQ(BigInt("5") % BigInt("7"), BigInt("5"));
}

TEST(BigIntDivision, ThrowsOnDivisionByZero) {
    EXPECT_THROW((void)(BigInt("5") / BigInt("0")), std::domain_error);
    EXPECT_THROW((void)(BigInt("5") % BigInt("0")), std::domain_error);
}

TEST(BigIntArithmeticProperties, SubtractionUndoesAddition) {
    std::mt19937_64 rng(1);
    std::uniform_int_distribution<int> length(1, 200);

    for (int round = 0; round < 200; ++round) {
        const BigInt lhs(test::randomDigits(rng, length(rng)));
        const BigInt rhs(test::randomDigits(rng, length(rng)));

        ASSERT_EQ((lhs + rhs) - rhs, lhs);
    }
}

TEST(BigIntArithmeticProperties, RemainderIsSmallerThanDivisorAndDividesTheRest) {
    std::mt19937_64 rng(2);
    std::uniform_int_distribution<int> length(1, 200);

    for (int round = 0; round < 200; ++round) {
        const BigInt dividend(test::randomDigits(rng, length(rng)));
        const BigInt divisor(test::randomDigits(rng, length(rng)));

        const BigInt remainder = dividend % divisor;

        ASSERT_LT(remainder, divisor);
        ASSERT_EQ((dividend - remainder) % divisor, BigInt("0"));
    }
}

}
