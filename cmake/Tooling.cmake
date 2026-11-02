# Targets de formatação e análise estática (clang-format / clang-tidy).
# Uso:
#   cmake --build --preset debug --target check         # só verifica
#   cmake --build --preset debug --target format        # formata os arquivos
#   cmake --build --preset debug --target tidy          # roda o clang-tidy
#   cmake --build --preset debug --target lint          # verifica tudo (check + tidy)
#   cmake --build --preset debug --target fix           # corrige tudo (tidy-fix + format)

file(GLOB_RECURSE BIGNUM_SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/include/*.hpp
    ${CMAKE_SOURCE_DIR}/src/*.hpp
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/app/*.hpp
    ${CMAKE_SOURCE_DIR}/app/*.cpp
    ${CMAKE_SOURCE_DIR}/tests/*.hpp
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
)

# clang-tidy só analisa unidades de tradução; cabeçalhos entram via includes.
set(BIGNUM_TIDY_FILES ${BIGNUM_SOURCE_FILES})
list(FILTER BIGNUM_TIDY_FILES INCLUDE REGEX "\\.cpp$")

find_program(CLANG_FORMAT_EXE NAMES clang-format)
find_program(CLANG_TIDY_EXE NAMES clang-tidy)

if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${BIGNUM_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatando código com clang-format"
        VERBATIM
    )
    add_custom_target(check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${BIGNUM_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Verificando formatação com clang-format"
        VERBATIM
    )
else()
    message(STATUS "clang-format não encontrado: targets 'format' e 'check' desativados")
endif()

if(CLANG_TIDY_EXE)
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${BIGNUM_TIDY_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Analisando código com clang-tidy"
        VERBATIM
    )
    add_custom_target(tidy-fix
        COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} --fix ${BIGNUM_TIDY_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Aplicando correções do clang-tidy"
        VERBATIM
    )
else()
    message(STATUS "clang-tidy não encontrado: targets 'tidy' e 'tidy-fix' desativados")
endif()

# Targets que fazem tudo de uma vez. Os comandos rodam em sequência no mesmo
# target: o tidy-fix vem antes do format porque as correções podem bagunçar a formatação.
if(CLANG_FORMAT_EXE AND CLANG_TIDY_EXE)
    add_custom_target(lint
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${BIGNUM_SOURCE_FILES}
        COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${BIGNUM_TIDY_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Verificando formatação e rodando clang-tidy"
        VERBATIM
    )
    add_custom_target(fix
        COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} --fix ${BIGNUM_TIDY_FILES}
        COMMAND ${CLANG_FORMAT_EXE} -i ${BIGNUM_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Aplicando correções do clang-tidy e formatando"
        VERBATIM
    )
endif()
