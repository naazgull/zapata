set(PARSERS_JSON_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-parser-json SHARED)
add_dependencies(zapata-parser-json
  zapata-base
)
target_sources(zapata-parser-json
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONPtr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONElement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONTokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONObj.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONTokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONRegex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONArr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONLambda.cpp
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONLexer.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONTokenizer.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONinc.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/json.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONTokenizerimpl.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONLexerbase.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONClass.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONParser.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONLexerimpl.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONTokenizerbase.h
    ${PARSERS_JSON_INCLUDE_DIR}/zapata/json/JSONTokenizerLexer.h
)
target_include_directories(zapata-parser-json
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${PARSERS_JSON_INCLUDE_DIR}
  INTERFACE
    ${PARSERS_JSON_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-json
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
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

add_executable(zapata_test_parser_json)
add_dependencies(zapata_test_parser_json
  zapata-base
  zapata-parser-json
)
target_sources(zapata_test_parser_json
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_parser_json
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${PARSERS_JSON_INCLUDE_DIR}
)
target_link_libraries(zapata_test_parser_json
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
)
set_target_properties(zapata_test_parser_json
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
