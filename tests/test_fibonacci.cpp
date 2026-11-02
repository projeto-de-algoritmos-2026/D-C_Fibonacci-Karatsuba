#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "BigInt.hpp"
#include "Fibonacci.hpp"
#include "KaratsubaMultiplier.hpp"
#include "Multiplier.hpp"
#include "SchoolbookMultiplier.hpp"

namespace bignum {

namespace {

struct KnownTerm {
    std::uint64_t index;
    std::string digits;
};

struct MultiplierCase {
    std::string name;
    std::shared_ptr<const Multiplier> multiplier;
};

std::string caseName(const ::testing::TestParamInfo<MultiplierCase>& info) {
    return info.param.name;
}

class CountingMultiplier final : public Multiplier {
private:
    SchoolbookMultiplier m_inner;
    mutable int m_calls = 0;

public:
    [[nodiscard]] BigInt multiply(const BigInt& lhs, const BigInt& rhs) const override {
        ++m_calls;
        return m_inner.multiply(lhs, rhs);
    }

    [[nodiscard]] int calls() const noexcept {
        return m_calls;
    }
};

class FibonacciContract : public ::testing::TestWithParam<MultiplierCase> {
protected:
    [[nodiscard]] static BigInt compute(std::uint64_t index) {
        return Fibonacci(*GetParam().multiplier).compute(index);
    }

    [[nodiscard]] static BigInt multiply(const BigInt& lhs, const BigInt& rhs) {
        return GetParam().multiplier->multiply(lhs, rhs);
    }
};

}

TEST_P(FibonacciContract, StartsTheSequence) {
    const std::vector<std::string> expected = {"0",   "1",   "1",   "2",    "3",    "5",   "8",
                                               "13",  "21",  "34",  "55",   "89",   "144", "233",
                                               "377", "610", "987", "1597", "2584", "4181"};

    for (std::uint64_t index = 0; index < expected.size(); ++index) {
        ASSERT_EQ(compute(index).toString(), expected[index]) << "F(" << index << ")";
    }
}

TEST_P(FibonacciContract, CrossesLimbBoundaries) {
    // F(47) e F(48) passam de 32 bits; F(93) e F(94), de 64 bits
    const std::vector<KnownTerm> terms = {{47, "2971215073"},
                                          {48, "4807526976"},
                                          {93, "12200160415121876738"},
                                          {94, "19740274219868223167"},
                                          {186, "332825110087067562321196029789634457848"},
                                          {187, "538522340430300790495419781092981030533"}};

    for (const KnownTerm& term : terms) {
        ASSERT_EQ(compute(term.index).toString(), term.digits) << "F(" << term.index << ")";
    }
}

TEST_P(FibonacciContract, MatchesKnownLargeTerms) {
    EXPECT_EQ(compute(100).toString(), "354224848179261915075");
    EXPECT_EQ(compute(300).toString(),
              "222232244629420445529739893461909967206666939096499764990979600");
    EXPECT_EQ(compute(1000).toString(),
              "434665576869374564356885276750406258025646605173717804024817290895365554179490518904"
              "038798400792551692959225930803226347752096896232398733224711616429964409065331879382"
              "98969649928516003704476137795166849228875");
}

TEST_P(FibonacciContract, MatchesKnownSizeOfHugeTerm) {
    const BigInt term = compute(100000);
    const std::string digits = term.toString();

    EXPECT_EQ(digits.size(), 20899U);
    EXPECT_EQ(term.limbs().size(), 2170U);
    EXPECT_EQ(digits.substr(0, 30), "259740693472217241661550340212");
    EXPECT_EQ(digits.substr(digits.size() - 30), "289236362349895374653428746875");
}

TEST_P(FibonacciContract, AgreesWithSumOnlyVersion) {
    for (std::uint64_t index = 0; index <= 300; ++index) {
        ASSERT_EQ(compute(index), Fibonacci::computeIterative(index)) << "F(" << index << ")";
    }

    // Em torno de n = 3.000, F(n/2) passa do limite de 32 limbs e o Karatsuba começa a recursão
    for (const std::uint64_t index : {2047U, 2048U, 2949U, 2950U, 3000U, 4096U, 10000U}) {
        ASSERT_EQ(compute(index), Fibonacci::computeIterative(index)) << "F(" << index << ")";
    }
}

TEST_P(FibonacciContract, FollowsTheRecurrenceOnLargeIndexes) {
    for (const std::uint64_t index : {5000U, 12345U, 50000U}) {
        ASSERT_EQ(compute(index) + compute(index + 1), compute(index + 2));
    }
}

TEST_P(FibonacciContract, SatisfiesCassiniIdentity) {
    for (const std::uint64_t index : {1U, 2U, 3U, 10U, 11U, 999U, 1000U, 20000U, 20001U}) {
        const BigInt square = multiply(compute(index), compute(index));
        const BigInt neighbors = multiply(compute(index - 1), compute(index + 1));

        // F(n-1)·F(n+1) - F(n)² = (-1)^n
        if (index % 2 == 0) {
            ASSERT_EQ(neighbors, square + BigInt("1")) << "n = " << index;
        } else {
            ASSERT_EQ(neighbors + BigInt("1"), square) << "n = " << index;
        }
    }
}

TEST_P(FibonacciContract, TermDividesItsMultiples) {
    // Se m divide n, F(m) divide F(n)
    EXPECT_EQ(compute(60) % compute(12), BigInt("0"));
    EXPECT_EQ(compute(1000) % compute(250), BigInt("0"));
    EXPECT_NE(compute(1001) % compute(250), BigInt("0"));
}

TEST_P(FibonacciContract, LastDigitsRepeatWithPisanoPeriod) {
    // Os períodos de Pisano para 10 e 1000 são 60 e 1500
    for (const std::uint64_t index : {0U, 1U, 7U, 59U, 1000U}) {
        ASSERT_EQ(compute(index + 60) % BigInt("10"), compute(index) % BigInt("10"));
        ASSERT_EQ(compute(index + 1500) % BigInt("1000"), compute(index) % BigInt("1000"));
    }
}

INSTANTIATE_TEST_SUITE_P(
    Algoritmos, FibonacciContract,
    ::testing::Values(MultiplierCase{"Escolar", std::make_shared<SchoolbookMultiplier>()},
                      MultiplierCase{"Karatsuba", std::make_shared<KaratsubaMultiplier>()},
                      MultiplierCase{"KaratsubaLimite8", std::make_shared<KaratsubaMultiplier>(8)}),
    caseName);

TEST(FibonacciAlgorithms, SchoolbookAndKaratsubaGiveTheSameHugeTerm) {
    const SchoolbookMultiplier schoolbook;
    const KaratsubaMultiplier karatsuba;

    EXPECT_EQ(Fibonacci(schoolbook).compute(200000), Fibonacci(karatsuba).compute(200000));
}

TEST(FibonacciAlgorithms, UsesTheInjectedMultiplier) {
    const CountingMultiplier counting;

    EXPECT_EQ(Fibonacci(counting).compute(10), BigInt("55"));
    EXPECT_GT(counting.calls(), 0);
}

TEST(FibonacciIterative, StartsTheSequence) {
    EXPECT_EQ(Fibonacci::computeIterative(0), BigInt("0"));
    EXPECT_EQ(Fibonacci::computeIterative(1), BigInt("1"));
    EXPECT_EQ(Fibonacci::computeIterative(2), BigInt("1"));
    EXPECT_EQ(Fibonacci::computeIterative(10), BigInt("55"));
}

TEST(FibonacciIterative, MatchesKnownLargeTerm) {
    EXPECT_EQ(Fibonacci::computeIterative(100).toString(), "354224848179261915075");
}

}
