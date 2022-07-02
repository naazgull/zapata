set(NETWORK_WEBSOCKET_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-websocket SHARED)

target_sources(zapata-net-websocket
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/websocket.cpp
  INTERFACE
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata/net/websocket.h
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata/net/transport/websocket.h
)
target_include_directories(zapata-net-websocket
  INTERFACE
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata-net-websocket
  INTERFACE
    zapata-base
    zapata-parser-json
)

set_target_properties(zapata-net-websocket
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

add_library(zapata-net-websocket-plugin SHARED)

target_sources(zapata-net-websocket-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-websocket-plugin
  INTERFACE
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata-net-websocket-plugin
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-net-websocket
)

set_target_properties(zapata-net-websocket-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
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
