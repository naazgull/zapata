set(PARSERS_HTTP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-parser-http SHARED)

target_sources(zapata-parser-http
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPTokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPTokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPReq.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPElement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/http.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPRep.cpp
  INTERFACE
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPParser.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPTokenizerLexer.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPTokenizer.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPTokenizerimpl.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPLexer.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPLexerimpl.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPLexerbase.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPTokenizerbase.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/http.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPinc.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPObj.h
)
target_include_directories(zapata-parser-http
  INTERFACE
    ${PARSERS_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-http
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-uri
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

add_executable(zapata_test_parser_http)

target_sources(zapata_test_parser_http
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_parser_http
  PUBLIC
    ${PARSERS_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata_test_parser_http
  PUBLIC
    ssl
    zapata-base
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
)

set_target_properties(zapata_test_parser_http
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)
