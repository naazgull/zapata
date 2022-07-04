set(NETWORK_TCP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-tcp SHARED)

target_sources(zapata-net-tcp
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp.cpp
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/tcp.h
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/transport/tcp.h
)
target_include_directories(zapata-net-tcp
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-tcp
  INTERFACE
    zapata-base
    zapata-parser-json
)

set_target_properties(zapata-net-tcp
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-net-tcp-plugin SHARED)

target_sources(zapata-net-tcp-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-tcp-plugin
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-tcp-plugin
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-net-tcp
)

set_target_properties(zapata-net-tcp-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
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
