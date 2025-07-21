set(NETWORK_WEBSOCKET_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-websocket SHARED)
add_dependencies(zapata-net-websocket
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-net-transport
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
  zapata-net-tcp
  zapata-parser-json
  zapata-parser-uri
  zapata-net-websocket
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
    zapata-parser-json
    zapata-parser-uri
    zapata-net-websocket
)
set_target_properties(zapata-net-websocket-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-net-websocket zapata-net-websocket-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
