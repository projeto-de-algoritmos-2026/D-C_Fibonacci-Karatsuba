cmake_minimum_required(VERSION 3.21)
project(FibonacciKaratsuba VERSION 1.0.0 LANGUAGES CXX)

# Configurações do compilador
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Opções de compilação
include(cmake/CompilerOptions.cmake)

# Formatação e análise estática (format, check, tidy, tidy-fix, lint, fix)
include(cmake/Tooling.cmake)

# Biblioteca com a implementação (src/) e cabeçalhos públicos (include/)
add_library(bignum STATIC
    src/BigInt.cpp
    src/SchoolbookMultiplier.cpp
    src/KaratsubaMultiplier.cpp
    src/Fibonacci.cpp
)
target_include_directories(bignum PUBLIC ${CMAKE_SOURCE_DIR}/include/bignum)
apply_compiler_options(bignum)

# CLI: calcula F(n) com o multiplicador escolhido
add_executable(fibonacci app/main.cpp)
target_link_libraries(fibonacci PRIVATE bignum)
apply_compiler_options(fibonacci)

# Benchmark: escolar x Karatsuba, saída em CSV
add_executable(fibonacci_benchmark app/Benchmark.cpp)
target_link_libraries(fibonacci_benchmark PRIVATE bignum)
apply_compiler_options(fibonacci_benchmark)

# Opção para compilar os testes
option(BIGNUM_BUILD_TESTS "Compilar os testes" ON)

if(BIGNUM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
