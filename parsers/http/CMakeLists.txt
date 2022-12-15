set(PARSERS_HTTP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-parser-http SHARED)
add_dependencies(zapata-parser-http
  zapata-base
  zapata-io-stream
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-parser-http
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPTokenizerLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPTokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPReq.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPElement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPLexer.cpp
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
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPinc.h
    ${PARSERS_HTTP_INCLUDE_DIR}/zapata/http/HTTPObj.h
)
target_include_directories(zapata-parser-http
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    ${PARSERS_HTTP_INCLUDE_DIR}
  INTERFACE
    ${PARSERS_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata-parser-http
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
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

add_executable(zapata_test_parser_http)
add_dependencies(zapata_test_parser_http
  zapata-base
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
)
target_sources(zapata_test_parser_http
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_parser_http
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-socket,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    ${PARSERS_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata_test_parser_http
  PRIVATE
    ssl
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-socket,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-http,LIBRARIES>
)

set_target_properties(zapata_test_parser_http
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
