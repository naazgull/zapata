set(PARSERS_URI_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-parser-uri SHARED)

target_sources(zapata-parser-uri
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URIParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URITokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URITokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URILexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uri.cpp
  INTERFACE
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URIParser.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URITokenizer.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/uri.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URITokenizerLexer.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URILexerbase.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URILexer.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URITokenizerimpl.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URIinc.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URILexerimpl.h
    ${PARSERS_URI_INCLUDE_DIR}/zapata/uri/URITokenizerbase.h
)
target_include_directories(zapata-parser-uri
  INTERFACE
    ${PARSERS_URI_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-uri
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-parser-uri
)

set_target_properties(zapata-parser-uri
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
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

add_executable(zapata_test_parser_uri)

target_sources(zapata_test_parser_uri
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_parser_uri
  PUBLIC
    ${PARSERS_URI_INCLUDE_DIR}
)
target_link_libraries(zapata_test_parser_uri
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-uri
)

set_target_properties(zapata_test_parser_uri
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
