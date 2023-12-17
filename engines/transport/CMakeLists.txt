set(ENGINE_TRANSPORT_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-transport SHARED)
add_dependencies(zapata-engine-transport
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-engine-startup
)
target_sources(zapata-engine-transport
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine.cpp
  INTERFACE
    ${ENGINE_TRANSPORT_INCLUDE_DIR}/zapata/transport/engine.h
)
target_include_directories(zapata-engine-transport
  PRIVATE
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-engine-startup
    ${ENGINE_TRANSPORT_INCLUDE_DIR}
  INTERFACE
    ${ENGINE_TRANSPORT_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-transport
  PUBLIC
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-engine-startup
)
set_target_properties(zapata-engine-transport
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-engine-transport.so
)

add_library(zapata-engine-transport-plugin SHARED)
add_dependencies(zapata-engine-transport-plugin
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-engine-startup
    zapata-engine-transport
)
target_sources(zapata-engine-transport-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-engine-transport-plugin
  PRIVATE
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-engine-startup
    zapata-engine-transport
    ${ENGINE_TRANSPORT_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-transport-plugin
  PRIVATE
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-engine-startup
    zapata-engine-transport
)
set_target_properties(zapata-engine-transport-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-transport zapata-engine-transport-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_TRANSPORT_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
