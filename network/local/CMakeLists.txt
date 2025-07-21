set(NETWORK_LOCAL_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-local SHARED)
add_dependencies(zapata-net-local
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-local
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file.cpp
  INTERFACE
    ${NETWORK_LOCAL_INCLUDE_DIR}/zapata/net/local.h
    ${NETWORK_LOCAL_INCLUDE_DIR}/zapata/net/transport/local.h
)
target_include_directories(zapata-net-local
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
    ${NETWORK_LOCAL_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_LOCAL_INCLUDE_DIR}
)
target_link_libraries(zapata-net-local
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
)
set_target_properties(zapata-net-local
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-local.so
)

add_library(zapata-net-local-plugin SHARED)
add_dependencies(zapata-net-local-plugin
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-net-local
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-local-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-local-plugin
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
    ${NETWORK_LOCAL_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_LOCAL_INCLUDE_DIR}
)
target_link_libraries(zapata-net-local-plugin
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
    zapata-net-local
)
set_target_properties(zapata-net-local-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-net-local zapata-net-local-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_LOCAL_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_local_transport)
add_dependencies(zapata_test_local_transport
  zapata-base
  zapata-parser-json
  zapata-parser-uri
  zapata-net-transport
  zapata-net-local
)
target_sources(zapata_test_local_transport
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_local_transport
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
    ${NETWORK_LOCAL_INCLUDE_DIR}
)
target_link_libraries(zapata_test_local_transport
  PRIVATE
    ssl
    crypto
    zapata-base
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
    zapata-net-local
)
set_target_properties(zapata_test_local_transport
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
