set(PARSERS_FUNCTIONAL_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_program(RE2C_EXECUTABLE re2c REQUIRED)
find_program(BISON_EXECUTABLE bison REQUIRED)

set(FUNCTIONAL_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${FUNCTIONAL_GEN_DIR})

set(FUNCTIONAL_LEXER_RE ${CMAKE_CURRENT_SOURCE_DIR}/src/Functional.re)
set(FUNCTIONAL_LEXER_CPP ${FUNCTIONAL_GEN_DIR}/FunctionalLexer.re2c.cpp)

set(FUNCTIONAL_GRAMMAR_Y ${CMAKE_CURRENT_SOURCE_DIR}/src/Functional.y)
set(FUNCTIONAL_PARSER_CPP ${FUNCTIONAL_GEN_DIR}/FunctionalParser.bison.cpp)
set(FUNCTIONAL_PARSER_H ${FUNCTIONAL_GEN_DIR}/FunctionalParser.bison.h)

add_custom_command(
  OUTPUT ${FUNCTIONAL_PARSER_CPP} ${FUNCTIONAL_PARSER_H}
  COMMAND ${BISON_EXECUTABLE}
          -Wall -Werror=conflicts-sr -Werror=conflicts-rr
          --defines=${FUNCTIONAL_PARSER_H}
          -o ${FUNCTIONAL_PARSER_CPP}
          ${FUNCTIONAL_GRAMMAR_Y}
  DEPENDS ${FUNCTIONAL_GRAMMAR_Y}
  COMMENT "Generating Functional parser with bison"
  VERBATIM
)

add_custom_command(
  OUTPUT ${FUNCTIONAL_LEXER_CPP}
  COMMAND ${RE2C_EXECUTABLE}
          -o ${FUNCTIONAL_LEXER_CPP}
          ${FUNCTIONAL_LEXER_RE}
  DEPENDS ${FUNCTIONAL_LEXER_RE} ${FUNCTIONAL_PARSER_H}
  COMMENT "Generating Functional lexer with re2c"
  VERBATIM
)

add_custom_target(zapata-parser-functional-generated
  DEPENDS ${FUNCTIONAL_PARSER_CPP} ${FUNCTIONAL_PARSER_H} ${FUNCTIONAL_LEXER_CPP}
)

add_library(zapata-parser-functional SHARED)
add_dependencies(zapata-parser-functional
  zapata-base
  zapata-parser-json
  zapata-parser-functional-generated
)
target_sources(zapata-parser-functional
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalTokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalTokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Re2cFunctionalLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/functional.cpp
    ${FUNCTIONAL_PARSER_CPP}
    ${FUNCTIONAL_LEXER_CPP}
  INTERFACE
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalParser.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalTokenizer.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/functional.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalTokenizerLexer.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/Re2cFunctionalLexer.h
)
target_include_directories(zapata-parser-functional
  PRIVATE
    zapata-base
    zapata-parser-json
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}
    ${FUNCTIONAL_GEN_DIR}
  INTERFACE
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-functional
  PRIVATE
    zapata-base
    zapata-parser-json
)
set_target_properties(zapata-parser-functional
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-parser-functional.so
)

include(GNUInstallDirs)
install(TARGETS zapata-parser-functional
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_parser_functional)
add_dependencies(zapata_example_parser_functional
  zapata-base
  zapata-parser-json
  zapata-parser-functional
)
target_sources(zapata_example_parser_functional
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/parsing.cpp
)
target_include_directories(zapata_example_parser_functional
  PRIVATE
    zapata-base
    zapata-parser-json
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}
)
target_link_libraries(zapata_example_parser_functional
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
)
set_target_properties(zapata_example_parser_functional
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
