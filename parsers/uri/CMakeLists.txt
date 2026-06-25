set(PARSERS_URI_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_program(RE2C_EXECUTABLE re2c REQUIRED)
find_program(BISON_EXECUTABLE bison REQUIRED)

set(URI_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${URI_GEN_DIR})

set(URI_LEXER_RE ${CMAKE_CURRENT_SOURCE_DIR}/src/URI.re)
set(URI_LEXER_CPP ${URI_GEN_DIR}/URILexer.re2c.cpp)

set(URI_GRAMMAR_Y ${CMAKE_CURRENT_SOURCE_DIR}/src/URI.y)
set(URI_PARSER_CPP ${URI_GEN_DIR}/URIParser.bison.cpp)
set(URI_PARSER_H ${URI_GEN_DIR}/URIParser.bison.h)

add_custom_command(
  OUTPUT ${URI_PARSER_CPP} ${URI_PARSER_H}
  COMMAND ${BISON_EXECUTABLE}
          -Wall -Werror=conflicts-sr -Werror=conflicts-rr
          --defines=${URI_PARSER_H}
          -o ${URI_PARSER_CPP}
          ${URI_GRAMMAR_Y}
  DEPENDS ${URI_GRAMMAR_Y}
  COMMENT "Generating URI parser with bison"
  VERBATIM
)

add_custom_command(
  OUTPUT ${URI_LEXER_CPP}
  COMMAND ${RE2C_EXECUTABLE}
          -o ${URI_LEXER_CPP}
          ${URI_LEXER_RE}
  DEPENDS ${URI_LEXER_RE} ${URI_PARSER_H}
  COMMENT "Generating URI lexer with re2c"
  VERBATIM
)

add_custom_target(zapata-parser-uri-generated
  DEPENDS ${URI_PARSER_CPP} ${URI_PARSER_H} ${URI_LEXER_CPP}
)

add_library(zapata-parser-uri SHARED)
add_dependencies(zapata-parser-uri
  zapata-base
  zapata-parser-json
  zapata-parser-uri-generated
)
target_sources(zapata-parser-uri
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URIParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URITokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URITokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Re2cURILexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uri.cpp
    ${URI_PARSER_CPP}
    ${URI_LEXER_CPP}
  INTERFACE
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URIParser.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URITokenizer.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/uri.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URITokenizerLexer.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/Re2cURILexer.h
)
target_include_directories(zapata-parser-uri
  PRIVATE
    zapata-base
    zapata-parser-json
    ${PARSERS_URI_INCLUDE_DIR}
    ${URI_GEN_DIR}
  INTERFACE
    ${PARSERS_URI_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-uri
  PRIVATE
    zapata-base
    zapata-parser-json
)
set_target_properties(zapata-parser-uri
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-parser-uri.so
)

include(GNUInstallDirs)
install(TARGETS zapata-parser-uri
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${PARSERS_URI_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_parser_uri)
add_dependencies(zapata_example_parser_uri
  zapata-base
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata_example_parser_uri
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/parsing.cpp
)
target_include_directories(zapata_example_parser_uri
  PRIVATE
    zapata-base
    zapata-parser-json
    ${PARSERS_URI_INCLUDE_DIR}
)
target_link_libraries(zapata_example_parser_uri
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-uri
)
set_target_properties(zapata_example_parser_uri
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
