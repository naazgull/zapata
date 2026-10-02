set(PARSERS_URI_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# The bison/re2c parser artifacts are pre-generated and checked in:
#   include/zapata/uri/URIParser.bison.h
#   src/URIParser.bison.cpp
#   src/URILexer.re2c.cpp
# Regenerate them (requires bison + re2c) with:  fish parsers/uri/generate.fish

add_library(zapata-parser-uri SHARED)
add_dependencies(zapata-parser-uri
  zapata-base
  zapata-parser-json
)
target_sources(zapata-parser-uri
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URIParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URITokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URITokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Re2cURILexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uri.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URIParser.bison.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/URILexer.re2c.cpp
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
