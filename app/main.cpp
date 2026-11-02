#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Fibonacci.hpp"
#include "KaratsubaMultiplier.hpp"
#include "SchoolbookMultiplier.hpp"

namespace {

constexpr std::size_t PREVIEW_DIGITS = 30;

void printUsage() {
    std::cerr << "Uso: fibonacci <n> [escolar|karatsuba] [--completo]\n"
              << "  n            indice do termo (ex.: 1000000)\n"
              << "  escolar      multiplicacao escolar, O(n^2)\n"
              << "  karatsuba    multiplicacao de Karatsuba, O(n^1.585) (padrao)\n"
              << "  --completo   imprime todos os digitos (por padrao, so o inicio e o fim)\n";
}

std::uint64_t parseIndex(const std::string& text) {
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("n deve conter apenas digitos");
    }
    return std::stoull(text);
}

std::unique_ptr<bignum::Multiplier> createMultiplier(const std::string& algorithm) {
    if (algorithm == "escolar") {
        return std::make_unique<bignum::SchoolbookMultiplier>();
    }
    return std::make_unique<bignum::KaratsubaMultiplier>();
}

}

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv, argv + argc);

    if (args.size() < 2 || args.size() > 4) {
        printUsage();
        return 1;
    }

    std::uint64_t index = 0;
    try {
        index = parseIndex(args[1]);
    } catch (const std::exception&) {
        std::cerr << "n invalido: " << args[1] << '\n';
        printUsage();
        return 1;
    }

    std::string algorithm = "karatsuba";
    bool showAllDigits = false;

    for (std::size_t position = 2; position < args.size(); ++position) {
        if (args[position] == "--completo") {
            showAllDigits = true;
        } else if (args[position] == "escolar" || args[position] == "karatsuba") {
            algorithm = args[position];
        } else {
            std::cerr << "Argumento desconhecido: " << args[position] << '\n';
            printUsage();
            return 1;
        }
    }

    const std::unique_ptr<bignum::Multiplier> multiplier = createMultiplier(algorithm);
    const bignum::Fibonacci fibonacci(*multiplier);

    const auto start = std::chrono::steady_clock::now();
    const bignum::BigInt result = fibonacci.compute(index);
    const auto computed = std::chrono::steady_clock::now();
    const std::string digits = result.toString();
    const auto converted = std::chrono::steady_clock::now();

    if (showAllDigits || digits.size() <= 2 * PREVIEW_DIGITS) {
        std::cout << "F(" << index << ") = " << digits << '\n';
    } else {
        std::cout << "F(" << index << ") = " << digits.substr(0, PREVIEW_DIGITS) << "..."
                  << digits.substr(digits.size() - PREVIEW_DIGITS) << '\n';
    }

    using Milliseconds = std::chrono::duration<double, std::milli>;
    std::cout << "Digitos:        " << digits.size() << '\n'
              << "Limbs:          " << result.limbs().size() << '\n'
              << "Algoritmo:      " << algorithm << '\n'
              << "Tempo do F(n):  " << Milliseconds(computed - start).count() << " ms\n"
              << "Tempo decimal:  " << Milliseconds(converted - computed).count() << " ms\n";

    return 0;
}
