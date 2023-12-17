set(ENGINE_REST_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-rest SHARED)
add_dependencies(zapata-engine-rest
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
  zapata-engine-transport
  zapata-common-catalog
)
target_sources(zapata-engine-rest
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rest.cpp
  INTERFACE
    ${ENGINE_REST_INCLUDE_DIR}/zapata/rest.h
    ${ENGINE_REST_INCLUDE_DIR}/zapata/rest/rest.h
)
target_include_directories(zapata-engine-rest
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
    zapata-engine-transport
    zapata-common-catalog
    ${ENGINE_REST_INCLUDE_DIR}
  INTERFACE
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-rest
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
    zapata-engine-transport
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
    zapata-common-catalog
)
set_target_properties(zapata-engine-rest
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-engine-rest.so
)

add_library(zapata-engine-rest-plugin SHARED)
add_dependencies(zapata-engine-rest-plugin
  zapata-base
  zapata-engine-startup
  zapata-engine-transport
  zapata-engine-rest
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-common-catalog
)
target_sources(zapata-engine-rest-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-engine-rest-plugin
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
    zapata-engine-transport
    zapata-common-catalog
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-rest-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-engine-rest
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-engine-transport
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
    zapata-common-catalog
)
set_target_properties(zapata-engine-rest-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata-rest-example SHARED)
add_dependencies(zapata-rest-example
  zapata-base
  zapata-engine-startup
  zapata-engine-transport
  zapata-engine-rest
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-common-catalog
)
target_sources(zapata-rest-example
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata-rest-example
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
    zapata-engine-transport
    zapata-storage-sqlite
    zapata-storage-connector
    zapata-common-catalog
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-rest-example
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-engine-rest
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
    zapata-engine-transport
    zapata-common-catalog
)
set_target_properties(zapata-rest-example
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-rest zapata-engine-rest-plugin zapata-rest-example
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_REST_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
