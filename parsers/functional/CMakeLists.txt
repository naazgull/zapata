set(PARSERS_FUNCTIONAL_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-parser-functional SHARED)
add_dependencies(zapata-parser-functional
  zapata-base
  zapata-parser-json
)
target_sources(zapata-parser-functional
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalTokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FunctionalTokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/functional.cpp
  INTERFACE
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalTokenizerbase.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalParser.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalTokenizerimpl.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/Functionalinc.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalLexerbase.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/functional.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalTokenizer.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalLexerimpl.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalTokenizerLexer.h
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}/zapata/functional/FunctionalLexer.h
)
target_include_directories(zapata-parser-functional
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}
  INTERFACE
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-functional
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
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

add_executable(zapata_test_parser_functional)
add_dependencies(zapata_test_parser_functional
  zapata-base
  zapata-parser-json
  zapata-parser-functional
)
target_sources(zapata_test_parser_functional
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_parser_functional
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    ${PARSERS_FUNCTIONAL_INCLUDE_DIR}
)
target_link_libraries(zapata_test_parser_functional
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-functional,LIBRARIES>
)
set_target_properties(zapata_test_parser_functional
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
