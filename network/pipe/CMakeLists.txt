set(NETWORK_PIPE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-pipe SHARED)

target_sources(zapata-net-pipe
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipe.cpp
  INTERFACE
    ${NETWORK_PIPE_INCLUDE_DIR}/zapata/net/pipe.h
    ${NETWORK_PIPE_INCLUDE_DIR}/zapata/net/transport/pipe.h
)
target_include_directories(zapata-net-pipe
  INTERFACE
    ${NETWORK_PIPE_INCLUDE_DIR}
)
target_link_libraries(zapata-net-pipe
  INTERFACE
    zapata-base
    zapata-parser-json
)

set_target_properties(zapata-net-pipe
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

add_library(zapata-net-pipe-plugin SHARED)

target_sources(zapata-net-pipe-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-pipe-plugin
  INTERFACE
    ${NETWORK_PIPE_INCLUDE_DIR}
)
target_link_libraries(zapata-net-pipe-plugin
  INTERFACE
    zapata-base
    zapata-parser-json
    zapata-net-pipe
)

set_target_properties(zapata-net-pipe-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

include(GNUInstallDirs)
install(TARGETS zapata-net-pipe zapata-net-pipe-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_PIPE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
