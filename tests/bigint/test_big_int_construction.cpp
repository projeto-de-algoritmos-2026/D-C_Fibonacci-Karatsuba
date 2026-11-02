#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "BigInt.hpp"

namespace bignum {

TEST(BigIntToString, ReturnsOriginalDigits) {
    EXPECT_EQ(BigInt("123").toString(), "123");
}

TEST(BigIntToString, RemovesLeadingZeros) {
    EXPECT_EQ(BigInt("000123").toString(), "123");
}

TEST(BigIntToString, ZeroStaysZero) {
    EXPECT_EQ(BigInt("0").toString(), "0");
}

TEST(BigIntToString, AllZerosBecomesSingleZero) {
    EXPECT_EQ(BigInt("000").toString(), "0");
}

TEST(BigIntToString, KeepsZerosInTheMiddleAndEnd) {
    EXPECT_EQ(BigInt("1000").toString(), "1000");
    EXPECT_EQ(BigInt("0010100").toString(), "10100");
}

TEST(BigIntToString, KeepsDigitsAcrossDecimalChunks) {
    EXPECT_EQ(BigInt("1000000000").toString(), "1000000000");
    EXPECT_EQ(BigInt("999999999999999999").toString(), "999999999999999999");
}

TEST(BigIntToString, RoundTripsLargeNumbers) {
    const std::string digits = "340282366920938463463374607431768211456";
    EXPECT_EQ(BigInt(digits).toString(), digits);
}

TEST(BigIntConstruction, RejectsEmptyString) {
    EXPECT_THROW(BigInt(""), std::invalid_argument);
}

TEST(BigIntConstruction, RejectsNonDigitCharacters) {
    EXPECT_THROW(BigInt("12a"), std::invalid_argument);
    EXPECT_THROW(BigInt("1.0"), std::invalid_argument);
    EXPECT_THROW(BigInt(" 7"), std::invalid_argument);
}

TEST(BigIntConstruction, RejectsSigns) {
    EXPECT_THROW(BigInt("-5"), std::invalid_argument);
    EXPECT_THROW(BigInt("+1"), std::invalid_argument);
}

TEST(BigIntFromLimbs, UsesLittleEndianLimbs) {
    EXPECT_EQ(BigInt::fromLimbs({0, 1}), BigInt("4294967296"));
}

TEST(BigIntFromLimbs, RemovesLeadingZeroLimbs) {
    EXPECT_EQ(BigInt::fromLimbs({5, 0, 0}).limbs().size(), 1U);
}

TEST(BigIntFromLimbs, EmptyLimbsBecomeZero) {
    EXPECT_EQ(BigInt::fromLimbs({}), BigInt("0"));
}

}
