#pragma once

#include <random>
#include <string>

namespace bignum::test {

inline std::string randomDigits(std::mt19937_64& rng, int count) {
    std::uniform_int_distribution<int> leading(1, 9);
    std::uniform_int_distribution<int> trailing(0, 9);

    std::string digits(1, static_cast<char>('0' + leading(rng)));
    for (int position = 1; position < count; ++position) {
        digits += static_cast<char>('0' + trailing(rng));
    }
    return digits;
}

}
