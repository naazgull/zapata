set(NETWORK_PIPE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-pipe SHARED)
add_dependencies(zapata-net-pipe
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-pipe
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-pipe
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipe.cpp
  INTERFACE
    ${NETWORK_PIPE_INCLUDE_DIR}/zapata/net/pipe.h
    ${NETWORK_PIPE_INCLUDE_DIR}/zapata/net/transport/pipe.h
)
target_include_directories(zapata-net-pipe
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-pipe
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
    ${NETWORK_PIPE_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_PIPE_INCLUDE_DIR}
)
target_link_libraries(zapata-net-pipe
  PRIVATE
    zapata-base
    zapata-globals
    zapata-io-stream
    zapata-io-pipe
    zapata-parser-json
    zapata-parser-uri
    zapata-net-transport
)
set_target_properties(zapata-net-pipe
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-pipe.so
)

add_library(zapata-net-pipe-plugin SHARED)
add_dependencies(zapata-net-pipe-plugin
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-pipe
  zapata-lockfree
  zapata-net-transport
  zapata-net-pipe
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-pipe-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-pipe-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-pipe
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${NETWORK_PIPE_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_PIPE_INCLUDE_DIR}
)
target_link_libraries(zapata-net-pipe-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-pipe
    zapata-net-transport
    zapata-net-pipe
    zapata-parser-json
    zapata-parser-uri
)
set_target_properties(zapata-net-pipe-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
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
