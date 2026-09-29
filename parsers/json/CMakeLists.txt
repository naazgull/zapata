set(PARSERS_JSON_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# The bison/re2c parser artifacts are pre-generated and checked in:
#   include/zapata/json/JSONParser.bison.h
#   src/JSONParser.bison.cpp
#   src/JSONLexer.re2c.cpp
# Regenerate them (requires bison + re2c) with:  fish parsers/json/generate.fish

add_library(zapata-parser-json SHARED)
add_dependencies(zapata-parser-json
  zapata-base
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONParser.bison.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONLexer.re2c.cpp
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
