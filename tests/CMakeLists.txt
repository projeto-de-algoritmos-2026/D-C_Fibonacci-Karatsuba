include(FetchContent)

# Evita aviso sobre timestamp de arquivos baixados (CMake >= 3.24)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Baixa o Google Test automaticamente na configuração
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Executável de testes
add_executable(FibonacciKaratsubaTests
    bigint/test_big_int_comparison.cpp
    bigint/test_big_int_construction.cpp
    bigint/test_big_int_arithmetic.cpp
    test_multipliers.cpp
    test_fibonacci.cpp
)

# Permite #include "support/RandomDigits.hpp"
target_include_directories(FibonacciKaratsubaTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(FibonacciKaratsubaTests PRIVATE
    bignum
    GTest::gtest_main
)

apply_compiler_options(FibonacciKaratsubaTests)

# Registra cada TEST() no CTest; um teste que passe de 120 s falha em vez de travar
include(GoogleTest)
gtest_discover_tests(FibonacciKaratsubaTests PROPERTIES TIMEOUT 120)
