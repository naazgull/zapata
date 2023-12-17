set(NETWORK_UPNP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-upnp SHARED)
add_dependencies(zapata-net-upnp
  zapata-base
  zapata-io-stream
  zapata-io-socket
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
)
target_sources(zapata-net-upnp
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/upnp.cpp
  INTERFACE
    ${NETWORK_UPNP_INCLUDE_DIR}/zapata/net/upnp.h
    ${NETWORK_UPNP_INCLUDE_DIR}/zapata/net/transport/upnp.h
)
target_include_directories(zapata-net-upnp
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-net-transport
    ${NETWORK_UPNP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_UPNP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-upnp
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-net-transport
)
set_target_properties(zapata-net-upnp
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-upnp.so
)

add_library(zapata-net-upnp-plugin SHARED)
add_dependencies(zapata-net-upnp-plugin
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-net-upnp
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
)
target_sources(zapata-net-upnp-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-upnp-plugin
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
    ${NETWORK_UPNP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_UPNP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-upnp-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-net-transport
    zapata-net-upnp
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
) 
set_target_properties(zapata-net-upnp-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-net-upnp zapata-net-upnp-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_UPNP_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
