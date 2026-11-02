#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include "BigInt.hpp"
#include "Fibonacci.hpp"
#include "KaratsubaMultiplier.hpp"
#include "Multiplier.hpp"
#include "SchoolbookMultiplier.hpp"

namespace {

using Clock = std::chrono::steady_clock;

constexpr double DEFAULT_MAX_SECONDS = 5.0;
constexpr double MIN_MEASURE_SECONDS = 0.2;
constexpr unsigned RANDOM_SEED = 42;
constexpr std::size_t FIRST_LIMBS = 16;
constexpr std::size_t LAST_LIMBS = 65536;
constexpr std::uint64_t FIRST_INDEX = 1000;
constexpr std::uint64_t LAST_INDEX = 64'000'000;
constexpr bignum::Limb TOP_BIT = 0x80000000U;
constexpr int SECONDS_PRECISION = 9;
constexpr int EXPONENT_PRECISION = 3;
constexpr std::size_t EXPONENT_FIT_POINTS = 4;

struct Measurement {
    double size;
    double seconds;
};

struct Algorithm {
    std::string name;
    const bignum::Multiplier* multiplier;
};

bignum::BigInt randomNumber(std::mt19937& rng, std::size_t limbCount) {
    bignum::Limbs limbs(limbCount);
    for (auto& limb : limbs) {
        limb = static_cast<bignum::Limb>(rng());
    }
    limbs[limbCount - 1] |= TOP_BIT;
    return bignum::BigInt::fromLimbs(limbs);
}

template <typename Operation>
double averageSeconds(Operation operation) {
    int repetitions = 0;
    const auto start = Clock::now();
    double elapsed = 0;

    while (repetitions == 0 || elapsed < MIN_MEASURE_SECONDS) {
        operation();
        ++repetitions;
        elapsed = std::chrono::duration<double>(Clock::now() - start).count();
    }

    return elapsed / repetitions;
}

void printLine(const std::string& part, const std::string& algorithm, std::uint64_t size,
               std::size_t limbs, double seconds) {
    std::cout << part << ',' << algorithm << ',' << size << ',' << limbs << ','
              << std::setprecision(SECONDS_PRECISION) << seconds << '\n'
              << std::flush;
}

double logLogSlope(const std::vector<Measurement>& points) {
    const std::size_t count = std::min(points.size(), EXPONENT_FIT_POINTS);
    const auto first = std::next(points.end(), -static_cast<std::ptrdiff_t>(count));

    double sumX = 0;
    double sumY = 0;
    double sumXX = 0;
    double sumXY = 0;

    for (auto point = first; point != points.end(); ++point) {
        const double logSize = std::log(point->size);
        const double logSeconds = std::log(point->seconds);
        sumX += logSize;
        sumY += logSeconds;
        sumXX += logSize * logSize;
        sumXY += logSize * logSeconds;
    }

    const auto samples = static_cast<double>(count);
    return (samples * sumXY - sumX * sumY) / (samples * sumXX - sumX * sumX);
}

void printExponent(const std::string& name, const std::vector<Measurement>& points) {
    if (points.size() < 2) {
        return;
    }

    std::cout << "# expoente empirico " << name << ": " << std::setprecision(EXPONENT_PRECISION)
              << logLogSlope(points) << '\n';
}

void benchmarkMultiplication(const Algorithm& algorithm, double maxSeconds) {
    std::mt19937 rng(RANDOM_SEED);
    std::vector<Measurement> points;

    for (std::size_t limbs = FIRST_LIMBS; limbs <= LAST_LIMBS; limbs *= 2) {
        const bignum::BigInt lhs = randomNumber(rng, limbs);
        const bignum::BigInt rhs = randomNumber(rng, limbs);

        const double seconds =
            averageSeconds([&] { (void)algorithm.multiplier->multiply(lhs, rhs); });

        printLine("multiplicacao", algorithm.name, limbs * bignum::LIMB_BITS, limbs, seconds);
        points.push_back({static_cast<double>(limbs), seconds});

        if (seconds > maxSeconds) {
            break;
        }
    }

    printExponent(algorithm.name, points);
}

void benchmarkFibonacci(const Algorithm& algorithm, double maxSeconds) {
    const bignum::Fibonacci fibonacci(*algorithm.multiplier);
    std::vector<Measurement> points;

    for (std::uint64_t index = FIRST_INDEX; index <= LAST_INDEX; index *= 2) {
        std::size_t limbs = 0;
        const double seconds =
            averageSeconds([&] { limbs = fibonacci.compute(index).limbs().size(); });

        printLine("fibonacci", algorithm.name, index, limbs, seconds);
        points.push_back({static_cast<double>(index), seconds});

        if (seconds > maxSeconds) {
            break;
        }
    }

    printExponent(algorithm.name, points);
}

}

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv, argv + argc);
    const double maxSeconds = args.size() > 1 ? std::stod(args[1]) : DEFAULT_MAX_SECONDS;

    const bignum::SchoolbookMultiplier schoolbook;
    const bignum::KaratsubaMultiplier karatsuba;
    const std::array<Algorithm, 2> algorithms = {
        {{"escolar", &schoolbook}, {"karatsuba", &karatsuba}}};

    std::cout << "# Parte 1: multiplicacao de dois numeros com o mesmo tamanho\n"
              << "parte,algoritmo,bits,limbs,segundos\n";
    for (const Algorithm& algorithm : algorithms) {
        benchmarkMultiplication(algorithm, maxSeconds);
    }

    std::cout << "\n# Parte 2: F(n) por fast doubling\n"
              << "parte,algoritmo,n,limbs_do_resultado,segundos\n";
    for (const Algorithm& algorithm : algorithms) {
        benchmarkFibonacci(algorithm, maxSeconds);
    }

    return 0;
}
