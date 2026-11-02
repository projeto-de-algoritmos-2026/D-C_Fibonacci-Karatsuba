#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "BigInt.hpp"
#include "KaratsubaMultiplier.hpp"
#include "Multiplier.hpp"
#include "SchoolbookMultiplier.hpp"
#include "support/RandomDigits.hpp"

namespace bignum {

namespace {

constexpr Limb MAX_LIMB = 0xFFFFFFFFU;

struct MultiplierCase {
    std::string name;
    std::shared_ptr<const Multiplier> multiplier;
};

MultiplierCase schoolbookCase() {
    return {"Escolar", std::make_shared<SchoolbookMultiplier>()};
}

MultiplierCase karatsubaCase(std::size_t threshold) {
    return {"KaratsubaLimite" + std::to_string(threshold),
            std::make_shared<KaratsubaMultiplier>(threshold)};
}

std::string caseName(const ::testing::TestParamInfo<MultiplierCase>& info) {
    return info.param.name;
}

BigInt allOnes(std::size_t limbCount) {
    return BigInt::fromLimbs(Limbs(limbCount, MAX_LIMB));
}

// (B^n - 1)^2 = B^(2n) - 2·B^n + 1
BigInt squareOfAllOnes(std::size_t limbCount) {
    Limbs limbs(2 * limbCount, MAX_LIMB);
    limbs[0] = 1;
    for (std::size_t index = 1; index < limbCount; ++index) {
        limbs[index] = 0;
    }
    limbs[limbCount] = MAX_LIMB - 1;
    return BigInt::fromLimbs(limbs);
}

BigInt powerOfBase(std::size_t exponent) {
    Limbs limbs(exponent + 1, 0);
    limbs[exponent] = 1;
    return BigInt::fromLimbs(limbs);
}

class MultiplierContract : public ::testing::TestWithParam<MultiplierCase> {
protected:
    [[nodiscard]] static BigInt multiply(const BigInt& lhs, const BigInt& rhs) {
        return GetParam().multiplier->multiply(lhs, rhs);
    }
};

class KaratsubaAgainstSchoolbook : public ::testing::TestWithParam<std::size_t> {
protected:
    SchoolbookMultiplier m_oracle;
    KaratsubaMultiplier m_karatsuba{GetParam()};
};

}

TEST_P(MultiplierContract, MultipliesSmallNumbers) {
    EXPECT_EQ(multiply(BigInt("6"), BigInt("7")), BigInt("42"));
}

TEST_P(MultiplierContract, ZeroAbsorbs) {
    const BigInt large("123456789012345678901234567890");

    EXPECT_EQ(multiply(large, BigInt("0")), BigInt("0"));
    EXPECT_EQ(multiply(BigInt("0"), large), BigInt("0"));
    EXPECT_EQ(multiply(BigInt("0"), BigInt("0")), BigInt("0"));
}

TEST_P(MultiplierContract, OneIsNeutral) {
    const BigInt large("123456789012345678901234567890");

    EXPECT_EQ(multiply(large, BigInt("1")), large);
    EXPECT_EQ(multiply(BigInt("1"), large), large);
}

TEST_P(MultiplierContract, CarriesIntoNewLimb) {
    EXPECT_EQ(multiply(BigInt("4294967295"), BigInt("4294967295")), BigInt("18446744065119617025"));
    EXPECT_EQ(multiply(BigInt("18446744073709551615"), BigInt("18446744073709551615")),
              BigInt("340282366920938463426481119284349108225"));
}

TEST_P(MultiplierContract, MatchesKnownProducts) {
    EXPECT_EQ(multiply(BigInt("123456789012345678901234567890"),
                       BigInt("987654321098765432109876543210")),
              BigInt("121932631137021795226185032733622923332237463801111263526900"));

    // 3^200 · 7^90, valor conferido fora do projeto
    EXPECT_EQ(
        multiply(
            BigInt("265613988875874769338781322035779626829233452653394495974574961739"
                   "092490901302182994384699044001"),
            BigInt(
                "11450477594321044359340126713545146077054004823284978858214566372120240027249")),
        BigInt("304140702836144316635406448223100326104515044063300465216670241979120193921"
               "018857865023594554337273334924100232102810041765503857834034817834517702986"
               "9163561411344489983249"));
}

TEST_P(MultiplierContract, HandlesWorstCaseCarry) {
    for (const std::size_t limbCount : {1U, 2U, 3U, 31U, 32U, 33U, 64U, 65U, 128U, 257U}) {
        const BigInt value = allOnes(limbCount);

        ASSERT_EQ(multiply(value, value), squareOfAllOnes(limbCount)) << limbCount << " limbs";
    }
}

TEST_P(MultiplierContract, AddsExponentsOfPowersOfBase) {
    for (const std::size_t exponent : {1U, 16U, 32U, 33U, 100U}) {
        ASSERT_EQ(multiply(powerOfBase(exponent), powerOfBase(2 * exponent)),
                  powerOfBase(3 * exponent));
    }
}

TEST_P(MultiplierContract, MultipliesOperandsOfVeryDifferentSizes) {
    // (B^n - 1) · B^k tem os limbs de B^n - 1 deslocados k posições
    Limbs expected(200, 0);
    for (std::size_t index = 100; index < expected.size(); ++index) {
        expected[index] = MAX_LIMB;
    }

    EXPECT_EQ(multiply(allOnes(100), powerOfBase(100)), BigInt::fromLimbs(expected));
    EXPECT_EQ(multiply(powerOfBase(100), allOnes(100)), BigInt::fromLimbs(expected));
    EXPECT_EQ(multiply(allOnes(200), BigInt("1")), allOnes(200));
}

TEST_P(MultiplierContract, ProductHasNoLeadingZeroLimbs) {
    const BigInt product = multiply(powerOfBase(40), BigInt("2"));

    EXPECT_EQ(product.limbs().size(), 41U);
    EXPECT_EQ(multiply(powerOfBase(40), BigInt("0")).limbs().size(), 1U);
}

TEST_P(MultiplierContract, IsCommutative) {
    std::mt19937_64 rng(3);
    std::uniform_int_distribution<int> length(1, 600);

    for (int round = 0; round < 30; ++round) {
        const BigInt lhs(test::randomDigits(rng, length(rng)));
        const BigInt rhs(test::randomDigits(rng, length(rng)));

        ASSERT_EQ(multiply(lhs, rhs), multiply(rhs, lhs));
    }
}

TEST_P(MultiplierContract, DistributesOverAddition) {
    std::mt19937_64 rng(4);
    std::uniform_int_distribution<int> length(1, 600);

    for (int round = 0; round < 30; ++round) {
        const BigInt factor(test::randomDigits(rng, length(rng)));
        const BigInt lhs(test::randomDigits(rng, length(rng)));
        const BigInt rhs(test::randomDigits(rng, length(rng)));

        ASSERT_EQ(multiply(factor, lhs + rhs), multiply(factor, lhs) + multiply(factor, rhs));
    }
}

TEST_P(MultiplierContract, DivisionUndoesMultiplication) {
    std::mt19937_64 rng(5);
    std::uniform_int_distribution<int> length(1, 200);

    for (int round = 0; round < 30; ++round) {
        const BigInt lhs(test::randomDigits(rng, length(rng)));
        const BigInt rhs(test::randomDigits(rng, length(rng)));
        const BigInt product = multiply(lhs, rhs);

        ASSERT_EQ(product / rhs, lhs);
        ASSERT_EQ(product % rhs, BigInt("0"));
    }
}

INSTANTIATE_TEST_SUITE_P(Algoritmos, MultiplierContract,
                         ::testing::Values(schoolbookCase(), karatsubaCase(1), karatsubaCase(2),
                                           karatsubaCase(3), karatsubaCase(8), karatsubaCase(32),
                                           karatsubaCase(64)),
                         caseName);

TEST_P(KaratsubaAgainstSchoolbook, AgreesOnRandomNumbersOfSameLength) {
    std::mt19937_64 rng(6);
    std::uniform_int_distribution<int> length(1, 3000);

    for (int round = 0; round < 20; ++round) {
        const int digits = length(rng);
        const BigInt lhs(test::randomDigits(rng, digits));
        const BigInt rhs(test::randomDigits(rng, digits));

        ASSERT_EQ(m_karatsuba.multiply(lhs, rhs), m_oracle.multiply(lhs, rhs));
    }
}

TEST_P(KaratsubaAgainstSchoolbook, AgreesOnRandomNumbersOfDifferentLengths) {
    std::mt19937_64 rng(7);
    std::uniform_int_distribution<int> length(1, 3000);

    for (int round = 0; round < 20; ++round) {
        const BigInt lhs(test::randomDigits(rng, length(rng)));
        const BigInt rhs(test::randomDigits(rng, length(rng)));

        ASSERT_EQ(m_karatsuba.multiply(lhs, rhs), m_oracle.multiply(lhs, rhs));
    }
}

TEST_P(KaratsubaAgainstSchoolbook, AgreesAroundTheThreshold) {
    std::mt19937_64 rng(8);
    const std::size_t threshold = GetParam();
    const std::vector<std::size_t> sizes = {threshold, threshold + 1, 2 * threshold,
                                            2 * threshold + 1};

    for (const std::size_t lhsLimbs : sizes) {
        for (const std::size_t rhsLimbs : sizes) {
            Limbs lhs(lhsLimbs);
            Limbs rhs(rhsLimbs);
            for (Limb& limb : lhs) {
                limb = static_cast<Limb>(rng());
            }
            for (Limb& limb : rhs) {
                limb = static_cast<Limb>(rng());
            }

            ASSERT_EQ(m_karatsuba.multiply(BigInt::fromLimbs(lhs), BigInt::fromLimbs(rhs)),
                      m_oracle.multiply(BigInt::fromLimbs(lhs), BigInt::fromLimbs(rhs)));
        }
    }
}

TEST_P(KaratsubaAgainstSchoolbook, AgreesWhenMiddleLimbsAreZero) {
    Limbs sparse(150, 0);
    sparse.front() = MAX_LIMB;
    sparse.back() = MAX_LIMB;
    const BigInt value = BigInt::fromLimbs(sparse);

    EXPECT_EQ(m_karatsuba.multiply(value, value), m_oracle.multiply(value, value));
    EXPECT_EQ(m_karatsuba.multiply(value, allOnes(150)), m_oracle.multiply(value, allOnes(150)));
}

INSTANTIATE_TEST_SUITE_P(Limites, KaratsubaAgainstSchoolbook,
                         ::testing::Values(1U, 2U, 3U, 8U, 32U, 64U));

TEST(KaratsubaThreshold, UsesDefaultValue) {
    EXPECT_EQ(KaratsubaMultiplier().threshold(), KaratsubaMultiplier::DEFAULT_THRESHOLD);
}

TEST(KaratsubaThreshold, KeepsCustomValue) {
    EXPECT_EQ(KaratsubaMultiplier(8).threshold(), 8U);
}

TEST(KaratsubaThreshold, RejectsZero) {
    EXPECT_THROW(KaratsubaMultiplier(0), std::invalid_argument);
}

}
