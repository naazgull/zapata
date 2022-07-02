set(NETWORK_HTTP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-http SHARED)

target_sources(zapata-net-http
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/http.cpp
  INTERFACE
    ${NETWORK_HTTP_INCLUDE_DIR}/zapata/net/http.h
    ${NETWORK_HTTP_INCLUDE_DIR}/zapata/net/transport/http.h
)
target_include_directories(zapata-net-http
  INTERFACE
    ${NETWORK_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-http
  INTERFACE
    zapata-base
    zapata-parser-json
)

set_target_properties(zapata-net-http
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

add_library(zapata-net-http-plugin SHARED)

target_sources(zapata-net-http-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-http-plugin
  INTERFACE
    ${NETWORK_HTTP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-http-plugin
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-net-http
)

set_target_properties(zapata-net-http-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
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
