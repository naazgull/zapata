set(PARSERS_FUNCTIONAL_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# The bison/re2c parser artifacts are pre-generated and checked in:
#   include/zapata/functional/FunctionalParser.bison.h
#   src/FunctionalParser.bison.cpp
#   src/FunctionalLexer.re2c.cpp
# Regenerate them (requires bison + re2c) with:  fish parsers/functional/generate.fish

add_library(zapata-parser-functional SHARED)
add_dependencies(zapata-parser-functional
  zapata-base
  zapata-parser-json
)
target_sources(zapata-parser-functional
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalTokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalTokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Re2cFunctionalLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/functional.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalParser.bison.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalLexer.re2c.cpp
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
