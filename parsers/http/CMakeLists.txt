set(PARSERS_HTTP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_program(RE2C_EXECUTABLE re2c REQUIRED)
find_program(BISON_EXECUTABLE bison REQUIRED)

set(HTTP_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${HTTP_GEN_DIR})

set(HTTP_LEXER_RE ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTP.re)
set(HTTP_LEXER_CPP ${HTTP_GEN_DIR}/HTTPLexer.re2c.cpp)

set(HTTP_GRAMMAR_Y ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTP.y)
set(HTTP_PARSER_CPP ${HTTP_GEN_DIR}/HTTPParser.bison.cpp)
set(HTTP_PARSER_H ${HTTP_GEN_DIR}/HTTPParser.bison.h)

add_custom_command(
  OUTPUT ${HTTP_PARSER_CPP} ${HTTP_PARSER_H}
  COMMAND ${BISON_EXECUTABLE}
          -Wall -Werror=conflicts-sr -Werror=conflicts-rr
          --defines=${HTTP_PARSER_H}
          -o ${HTTP_PARSER_CPP}
          ${HTTP_GRAMMAR_Y}
  DEPENDS ${HTTP_GRAMMAR_Y}
  COMMENT "Generating HTTP parser with bison"
  VERBATIM
)

add_custom_command(
  OUTPUT ${HTTP_LEXER_CPP}
  COMMAND ${RE2C_EXECUTABLE}
          -o ${HTTP_LEXER_CPP}
          ${HTTP_LEXER_RE}
  DEPENDS ${HTTP_LEXER_RE} ${HTTP_PARSER_H}
  COMMENT "Generating HTTP lexer with re2c"
  VERBATIM
)

add_custom_target(zapata-parser-http-generated
  DEPENDS ${HTTP_PARSER_CPP} ${HTTP_PARSER_H} ${HTTP_LEXER_CPP}
)

add_library(zapata-parser-http SHARED)
add_dependencies(zapata-parser-http
  zapata-base
  zapata-io-stream
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http-generated
)
target_sources(zapata-parser-http
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPTokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPTokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Re2cHTTPLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPReq.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPElement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPRep.cpp
    ${HTTP_PARSER_CPP}
    ${HTTP_LEXER_CPP}
  INTERFACE
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPParser.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPTokenizerLexer.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPTokenizer.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/Re2cHTTPLexer.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPObj.h
)
target_include_directories(zapata-parser-http
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${PARSERS_HTTP_INCLUDE_DIR}
    ${HTTP_GEN_DIR}
  INTERFACE
    ${PARSERS_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-http
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
)
set_target_properties(zapata-parser-http
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-parser-http.so
)

include(GNUInstallDirs)
install(TARGETS zapata-parser-http
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${PARSERS_HTTP_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_http_server)
add_dependencies(zapata_example_http_server
  zapata-base
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
)
target_sources(zapata_example_http_server
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/server.cpp
)
target_include_directories(zapata_example_http_server
  PRIVATE
    zapata-base
    zapata-io-socket
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${PARSERS_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata_example_http_server
  PRIVATE
    ssl
    zapata-base
    zapata-io-socket
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
)

set_target_properties(zapata_example_http_server
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_executable(zapata_example_http_parser)
add_dependencies(zapata_example_http_parser
  zapata-base
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
)
target_sources(zapata_example_http_parser
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/messages.cpp
)
target_include_directories(zapata_example_http_parser
  PRIVATE
    zapata-base
    zapata-io-socket
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${PARSERS_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata_example_http_parser
  PRIVATE
    ssl
    zapata-base
    zapata-io-socket
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
)

set_target_properties(zapata_example_http_parser
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
