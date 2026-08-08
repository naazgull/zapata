set(NETWORK_WEBSOCKET_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(SQLite3 QUIET)

if(NOT SQLite3_FOUND)
  message(STATUS "zapata-net-websocket skipped: SQLite3 not found")
  return()
endif()

add_library(zapata-net-websocket SHARED)
add_dependencies(zapata-net-websocket
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-net-transport
  zapata-parser-http
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-websocket
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/websocket.cpp
  INTERFACE
  ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata/net/websocket.h
  ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata/net/transport/websocket.h
)
target_include_directories(zapata-net-websocket
  PRIVATE
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-parser-http
  zapata-parser-json
  zapata-parser-uri
  zapata-net-transport
  ${NETWORK_WEBSOCKET_INCLUDE_DIR}
  INTERFACE
  ${NETWORK_WEBSOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata-net-websocket
  PRIVATE
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-parser-http
  zapata-parser-json
  zapata-parser-uri
  zapata-net-transport
)
set_target_properties(zapata-net-websocket
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
  LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
  LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-websocket.so
)

add_library(zapata-net-websocket-plugin SHARED)
add_dependencies(zapata-net-websocket-plugin
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-net-websocket
  zapata-parser-http
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-websocket-plugin
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-websocket-plugin
  PRIVATE
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-parser-http
  zapata-parser-json
  zapata-parser-uri
  ${NETWORK_WEBSOCKET_INCLUDE_DIR}
  INTERFACE
  ${NETWORK_WEBSOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata-net-websocket-plugin
  PRIVATE
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-net-transport
  zapata-net-websocket
  zapata-parser-http
  zapata-parser-json
  zapata-parser-uri
)
set_target_properties(zapata-net-websocket-plugin
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
  LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata-example-ws-provider SHARED)
add_dependencies(zapata-example-ws-provider
  zapata-base
  zapata-engine-startup
  zapata-engine-transport
  zapata-engine-rest
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-common-catalog
)
target_sources(zapata-example-ws-provider
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/provider.cpp
)
target_include_directories(zapata-example-ws-provider
  PRIVATE
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-engine-transport
  zapata-storage-sqlite
  zapata-storage-connector
  zapata-common-catalog
  ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-example-ws-provider
  PRIVATE
  zapata-base
  zapata-engine-startup
  zapata-engine-rest
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-storage-connector
  ${SQLite3_LIBRARIES}
  zapata-storage-sqlite
  zapata-engine-transport
  zapata-common-catalog
)
set_target_properties(zapata-example-ws-provider
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
  LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata-example-ws-consumer SHARED)
add_dependencies(zapata-example-ws-consumer
  zapata-base
  zapata-engine-startup
  zapata-engine-transport
  zapata-engine-rest
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-common-catalog
)
target_sources(zapata-example-ws-consumer
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/consumer.cpp
)
target_include_directories(zapata-example-ws-consumer
  PRIVATE
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-engine-transport
  zapata-storage-sqlite
  zapata-storage-connector
  zapata-common-catalog
  ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-example-ws-consumer
  PRIVATE
  zapata-base
  zapata-engine-startup
  zapata-engine-rest
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-storage-connector
  ${SQLite3_LIBRARIES}
  zapata-storage-sqlite
  zapata-engine-transport
  zapata-common-catalog
)
set_target_properties(zapata-example-ws-consumer
  PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
  LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(
  TARGETS
    zapata-net-websocket
    zapata-net-websocket-plugin
    zapata-example-ws-provider
    zapata-example-ws-consumer
  LIBRARY
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
