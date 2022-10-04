set(NETWORK_TCP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-tcp SHARED)
add_dependencies(zapata-net-tcp
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-tcp
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp.cpp
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/tcp.h
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/transport/tcp.h
)
target_include_directories(zapata-net-tcp
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-socket,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    ${NETWORK_TCP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-tcp
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-globals,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-socket,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
)
set_target_properties(zapata-net-tcp
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-tcp.so
)

add_library(zapata-net-tcp-plugin SHARED)
add_dependencies(zapata-net-tcp-plugin
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-net-tcp
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-tcp-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-tcp-plugin
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-engine-startup,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-events,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-socket,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    ${NETWORK_TCP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-tcp-plugin
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-engine-startup,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
    $<TARGET_PROPERTY:zapata-globals,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-socket,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-tcp,LIBRARIES>
 )
set_target_properties(zapata-net-tcp-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-net-tcp zapata-net-tcp-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_TCP_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_tcp_transport)
add_dependencies(zapata_test_tcp_transport
  zapata-base
  zapata-parser-json
  zapata-parser-uri
  zapata-net-transport
  zapata-net-tcp
)
target_sources(zapata_test_tcp_transport
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_tcp_transport
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-socket,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata_test_tcp_transport
  PRIVATE
    ssl
    crypto
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-socket,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-tcp,LIBRARIES>
)
set_target_properties(zapata_test_tcp_transport
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
