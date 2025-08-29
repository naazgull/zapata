set(NETWORK_SELF_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-self SHARED)
add_dependencies(zapata-net-self
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-self
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/self.cpp
  INTERFACE
    ${NETWORK_SELF_INCLUDE_DIR}/zapata/net/self.h
    ${NETWORK_SELF_INCLUDE_DIR}/zapata/net/transport/self.h
)
target_include_directories(zapata-net-self
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${NETWORK_SELF_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_SELF_INCLUDE_DIR}
)
target_link_libraries(zapata-net-self
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
)
set_target_properties(zapata-net-self
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-self.so
)

add_library(zapata-net-self-plugin SHARED)
add_dependencies(zapata-net-self-plugin
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-lockfree
  zapata-net-transport
  zapata-net-self
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-self-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-self-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${NETWORK_SELF_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_SELF_INCLUDE_DIR}
)
target_link_libraries(zapata-net-self-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-net-self
 )
set_target_properties(zapata-net-self-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-net-self zapata-net-self-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_SELF_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_self_transport)
add_dependencies(zapata_test_self_transport
  zapata-base
  zapata-io-stream
  zapata-net-transport
  zapata-net-self
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata_test_self_transport
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_self_transport
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${NETWORK_SELF_INCLUDE_DIR}
)
target_link_libraries(zapata_test_self_transport
  PRIVATE
    ssl
    crypto
    zapata-base
    zapata-io-stream
    zapata-net-transport
    zapata-net-self
    zapata-parser-json
    zapata-parser-uri
)
set_target_properties(zapata_test_self_transport
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
