set(NETWORK_LOCAL_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-local SHARED)

target_sources(zapata-net-local
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file.cpp
  INTERFACE
    ${NETWORK_LOCAL_INCLUDE_DIR}/zapata/net/local.h
    ${NETWORK_LOCAL_INCLUDE_DIR}/zapata/net/transport/local.h
)
target_include_directories(zapata-net-local
  INTERFACE
    ${NETWORK_LOCAL_INCLUDE_DIR}
)
target_link_libraries(zapata-net-local
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-net-transport
)

set_target_properties(zapata-net-local
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

add_library(zapata-net-local-plugin SHARED)

target_sources(zapata-net-local-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-local-plugin
  INTERFACE
    ${NETWORK_LOCAL_INCLUDE_DIR}
)
target_link_libraries(zapata-net-local-plugin
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-net-transport
    zapata-net-local
)

set_target_properties(zapata-net-local-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
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