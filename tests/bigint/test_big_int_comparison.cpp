#include <gtest/gtest.h>

#include "BigInt.hpp"

namespace bignum {

TEST(BigIntEquality, TrueForSameValue) {
    EXPECT_TRUE(BigInt("123") == BigInt("123"));
}

TEST(BigIntEquality, FalseForDifferentValues) {
    EXPECT_FALSE(BigInt("123") == BigInt("124"));
}

TEST(BigIntEquality, IgnoresLeadingZeros) {
    EXPECT_TRUE(BigInt("000123") == BigInt("123"));
}

TEST(BigIntEquality, ZeroEqualsAnyAmountOfZeros) {
    EXPECT_TRUE(BigInt("0") == BigInt("000"));
}

TEST(BigIntEquality, FalseWhenOnlyLengthDiffers) {
    EXPECT_FALSE(BigInt("1") == BigInt("10"));
}

TEST(BigIntNotEqual, TrueForDifferentValues) {
    EXPECT_TRUE(BigInt("5") != BigInt("6"));
}

TEST(BigIntNotEqual, FalseForSameValue) {
    EXPECT_FALSE(BigInt("5") != BigInt("5"));
}

TEST(BigIntNotEqual, FalseWhenOnlyLeadingZerosDiffer) {
    EXPECT_FALSE(BigInt("0005") != BigInt("5"));
}

TEST(BigIntNotEqual, FalseForZeroVsManyZeros) {
    EXPECT_FALSE(BigInt("0") != BigInt("0000"));
}

TEST(BigIntNotEqual, TrueWhenOnlyLengthDiffers) {
    EXPECT_TRUE(BigInt("1") != BigInt("10"));
}

TEST(BigIntLessThan, ComparesByLength) {
    EXPECT_TRUE(BigInt("99") < BigInt("100"));
}

TEST(BigIntLessThan, ComparesDigitByDigitWhenSameLength) {
    EXPECT_TRUE(BigInt("123") < BigInt("124"));
}

TEST(BigIntLessThan, FalseForEqualValues) {
    EXPECT_FALSE(BigInt("123") < BigInt("123"));
}

TEST(BigIntLessThan, FalseWhenLeftIsGreater) {
    EXPECT_FALSE(BigInt("124") < BigInt("123"));
    EXPECT_FALSE(BigInt("100") < BigInt("99"));
}

TEST(BigIntLessThan, NineIsLessThanTen) {
    EXPECT_TRUE(BigInt("9") < BigInt("10"));
    EXPECT_FALSE(BigInt("10") < BigInt("9"));
}

TEST(BigIntLessThan, DifferenceInFirstDigitDecides) {
    EXPECT_TRUE(BigInt("199") < BigInt("299"));
}

TEST(BigIntLessThan, ZeroIsLessThanAnyPositive) {
    EXPECT_TRUE(BigInt("0") < BigInt("1"));
    EXPECT_FALSE(BigInt("1") < BigInt("0"));
}

TEST(BigIntLessThan, ZeroIsNotLessThanZeroWithLeadingZeros) {
    EXPECT_FALSE(BigInt("0") < BigInt("000"));
}

TEST(BigIntLessThan, IgnoresLeadingZeros) {
    EXPECT_TRUE(BigInt("0099") < BigInt("100"));
    EXPECT_FALSE(BigInt("0100") < BigInt("99"));
}

TEST(BigIntGreaterThan, ComparesByLength) {
    EXPECT_TRUE(BigInt("100") > BigInt("99"));
}

TEST(BigIntGreaterThan, ComparesDigitByDigitWhenSameLength) {
    EXPECT_TRUE(BigInt("124") > BigInt("123"));
}

TEST(BigIntGreaterThan, FalseForEqualValues) {
    EXPECT_FALSE(BigInt("123") > BigInt("123"));
}

TEST(BigIntGreaterThan, FalseWhenLeftIsSmaller) {
    EXPECT_FALSE(BigInt("123") > BigInt("124"));
    EXPECT_FALSE(BigInt("99") > BigInt("100"));
}

TEST(BigIntGreaterThan, TenIsGreaterThanNine) {
    EXPECT_TRUE(BigInt("10") > BigInt("9"));
    EXPECT_FALSE(BigInt("9") > BigInt("10"));
}

TEST(BigIntGreaterThan, AnyPositiveIsGreaterThanZero) {
    EXPECT_TRUE(BigInt("1") > BigInt("0"));
    EXPECT_FALSE(BigInt("0") > BigInt("1"));
}

TEST(BigIntGreaterThan, ZeroIsNotGreaterThanZeroWithLeadingZeros) {
    EXPECT_FALSE(BigInt("000") > BigInt("0"));
}

TEST(BigIntGreaterThan, IgnoresLeadingZeros) {
    EXPECT_TRUE(BigInt("0100") > BigInt("99"));
    EXPECT_FALSE(BigInt("0099") > BigInt("100"));
}

TEST(BigIntLessOrEqual, TrueForEqualValues) {
    EXPECT_TRUE(BigInt("5") <= BigInt("5"));
}

TEST(BigIntLessOrEqual, TrueWhenLeftIsLess) {
    EXPECT_TRUE(BigInt("4") <= BigInt("5"));
    EXPECT_TRUE(BigInt("99") <= BigInt("100"));
}

TEST(BigIntLessOrEqual, FalseWhenLeftIsGreater) {
    EXPECT_FALSE(BigInt("6") <= BigInt("5"));
    EXPECT_FALSE(BigInt("100") <= BigInt("99"));
}

TEST(BigIntLessOrEqual, TrueForEqualValuesWithDifferentLeadingZeros) {
    EXPECT_TRUE(BigInt("005") <= BigInt("5"));
    EXPECT_TRUE(BigInt("5") <= BigInt("005"));
}

TEST(BigIntLessOrEqual, ZeroIsLessOrEqualToZero) {
    EXPECT_TRUE(BigInt("0") <= BigInt("000"));
}

TEST(BigIntLessOrEqual, NineIsLessOrEqualToTen) {
    EXPECT_TRUE(BigInt("9") <= BigInt("10"));
    EXPECT_FALSE(BigInt("10") <= BigInt("9"));
}

TEST(BigIntGreaterOrEqual, TrueForEqualValues) {
    EXPECT_TRUE(BigInt("5") >= BigInt("5"));
}

TEST(BigIntGreaterOrEqual, TrueWhenLeftIsGreater) {
    EXPECT_TRUE(BigInt("6") >= BigInt("5"));
    EXPECT_TRUE(BigInt("100") >= BigInt("99"));
}

TEST(BigIntGreaterOrEqual, FalseWhenLeftIsLess) {
    EXPECT_FALSE(BigInt("4") >= BigInt("5"));
    EXPECT_FALSE(BigInt("99") >= BigInt("100"));
}

TEST(BigIntGreaterOrEqual, TrueForEqualValuesWithDifferentLeadingZeros) {
    EXPECT_TRUE(BigInt("005") >= BigInt("5"));
    EXPECT_TRUE(BigInt("5") >= BigInt("005"));
}

TEST(BigIntGreaterOrEqual, ZeroIsGreaterOrEqualToZero) {
    EXPECT_TRUE(BigInt("000") >= BigInt("0"));
}

TEST(BigIntGreaterOrEqual, TenIsGreaterOrEqualToNine) {
    EXPECT_TRUE(BigInt("10") >= BigInt("9"));
    EXPECT_FALSE(BigInt("9") >= BigInt("10"));
}

}
