set(NETWORK_TRANSPORT_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-transport SHARED)
add_dependencies(zapata-net-transport
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-transport
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transport.cpp
  INTERFACE
    ${NETWORK_TRANSPORT_INCLUDE_DIR}/zapata/transport.h
    ${NETWORK_TRANSPORT_INCLUDE_DIR}/zapata/transport/transport.h
)
target_include_directories(zapata-net-transport
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    ${NETWORK_TRANSPORT_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TRANSPORT_INCLUDE_DIR}
)
target_link_libraries(zapata-net-transport
  PUBLIC
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
)
set_target_properties(zapata-net-transport
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-transport.so
)

include(GNUInstallDirs)
install(TARGETS zapata-net-transport
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_TRANSPORT_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_network_transport)
add_dependencies(zapata_test_network_transport
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-parser-json
  zapata-parser-uri
  zapata-net-transport
)
target_sources(zapata_test_network_transport
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_network_transport
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
    ${NETWORK_TRANSPORT_INCLUDE_DIR}
)
target_link_libraries(zapata_test_network_transport
  PRIVATE
    ssl
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
)
set_target_properties(zapata_test_network_transport
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
