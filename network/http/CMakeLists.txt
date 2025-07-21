set(NETWORK_HTTP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-http SHARED)
add_dependencies(zapata-net-http
  zapata-base
  zapata-io-stream
  zapata-io-socket
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
)
target_sources(zapata-net-http
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/http.cpp
  INTERFACE
    ${NETWORK_HTTP_INCLUDE_DIR}/zapata/net/http.h
    ${NETWORK_HTTP_INCLUDE_DIR}/zapata/net/transport/http.h
)
target_include_directories(zapata-net-http
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-net-transport
    ${NETWORK_HTTP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-http
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-net-transport
)
set_target_properties(zapata-net-http
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-http.so
)

add_library(zapata-net-http-plugin SHARED)
add_dependencies(zapata-net-http-plugin
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-net-http
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
)
target_sources(zapata-net-http-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-http-plugin
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
    zapata-parser-http
    ${NETWORK_HTTP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-http-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-net-transport
    zapata-net-http
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
) 
set_target_properties(zapata-net-http-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-net-http zapata-net-http-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_HTTP_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
