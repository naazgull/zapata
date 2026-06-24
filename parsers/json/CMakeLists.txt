set(PARSERS_JSON_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_program(RE2C_EXECUTABLE re2c REQUIRED)
find_program(BISON_EXECUTABLE bison REQUIRED)

set(JSON_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${JSON_GEN_DIR})

set(JSON_LEXER_RE ${CMAKE_CURRENT_SOURCE_DIR}/src/JSON.re)
set(JSON_LEXER_CPP ${JSON_GEN_DIR}/JSONLexer.re2c.cpp)

set(JSON_GRAMMAR_Y ${CMAKE_CURRENT_SOURCE_DIR}/src/JSON.y)
set(JSON_PARSER_CPP ${JSON_GEN_DIR}/JSONParser.bison.cpp)
set(JSON_PARSER_H ${JSON_GEN_DIR}/JSONParser.bison.h)

add_custom_command(
  OUTPUT ${JSON_PARSER_CPP} ${JSON_PARSER_H}
  COMMAND ${BISON_EXECUTABLE}
          -Wall -Werror=conflicts-sr -Werror=conflicts-rr
          --defines=${JSON_PARSER_H}
          -o ${JSON_PARSER_CPP}
          ${JSON_GRAMMAR_Y}
  DEPENDS ${JSON_GRAMMAR_Y}
  COMMENT "Generating JSON parser with bison"
  VERBATIM
)

add_custom_command(
  OUTPUT ${JSON_LEXER_CPP}
  COMMAND ${RE2C_EXECUTABLE}
          -o ${JSON_LEXER_CPP}
          ${JSON_LEXER_RE}
  DEPENDS ${JSON_LEXER_RE} ${JSON_PARSER_H}
  COMMENT "Generating JSON lexer with re2c"
  VERBATIM
)

add_custom_target(zapata-parser-json-generated
  DEPENDS ${JSON_PARSER_CPP} ${JSON_PARSER_H} ${JSON_LEXER_CPP}
)

add_library(zapata-parser-json SHARED)
add_dependencies(zapata-parser-json
  zapata-base
  zapata-parser-json-generated
)
target_sources(zapata-parser-json
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONPtr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONElement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONTokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONObj.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONTokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Re2cJSONLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONRegex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONArr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONLambda.cpp
    ${JSON_PARSER_CPP}
    ${JSON_LEXER_CPP}
  INTERFACE
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/json.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONClass.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONParser.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONTokenizer.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONTokenizerLexer.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/Re2cJSONLexer.h
)
target_include_directories(zapata-parser-json
  PRIVATE
    zapata-base
    ${PARSERS_JSON_INCLUDE_DIR}
    ${JSON_GEN_DIR}
  INTERFACE
    ${PARSERS_JSON_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-json
  PRIVATE
    zapata-base
)
set_target_properties(zapata-parser-json
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-parser-json.so
)

include(GNUInstallDirs)
install(TARGETS zapata-parser-json
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${PARSERS_JSON_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_parser_json)
add_dependencies(zapata_example_parser_json
  zapata-base
  zapata-parser-json
)
target_sources(zapata_example_parser_json
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/parsing.cpp
)
target_include_directories(zapata_example_parser_json
  PRIVATE
    zapata-base
    ${PARSERS_JSON_INCLUDE_DIR}
)
target_link_libraries(zapata_example_parser_json
  PRIVATE
    zapata-base
    zapata-parser-json
)
set_target_properties(zapata_example_parser_json
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
