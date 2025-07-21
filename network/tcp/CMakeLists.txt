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
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp.cpp
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/tcp.h
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/transport/tcp.h
)
target_include_directories(zapata-net-tcp
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
    ${NETWORK_TCP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-tcp
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
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
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-tcp-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${NETWORK_TCP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-tcp-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-net-tcp
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
    zapata-base
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata_test_tcp_transport
  PRIVATE
    ssl
    crypto
    zapata-base
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
    zapata-net-tcp
)
set_target_properties(zapata_test_tcp_transport
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
