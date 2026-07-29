set(ENGINE_REST_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(SQLite3 QUIET)

if(NOT SQLite3_FOUND)
  message(STATUS "zapata-engine-rest skipped: SQLite3 not found")
  return()
endif()

add_library(zapata-engine-rest SHARED)
add_dependencies(zapata-engine-rest
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-self
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-engine-transport
  zapata-common-catalog
)
target_sources(zapata-engine-rest
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pending_messages.cpp
  INTERFACE
    ${ENGINE_REST_INCLUDE_DIR}/zapata/rest.h
    ${ENGINE_REST_INCLUDE_DIR}/zapata/rest/rest.h
    ${ENGINE_REST_INCLUDE_DIR}/zapata/rest/services.h
    ${ENGINE_REST_INCLUDE_DIR}/zapata/rest/pending_messages.h
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
    zapata-net-self
    zapata-net-transport
    zapata-ontology
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
    zapata-net-self
    zapata-net-transport
    zapata-ontology
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
  zapata-net-self
  zapata-net-transport
  zapata-ontology
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
    zapata-net-self
    zapata-net-transport
    zapata-ontology
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
    zapata-net-self
    zapata-net-transport
    zapata-ontology
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

add_library(zapata-example-rest-provider SHARED)
add_dependencies(zapata-example-rest-provider
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
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-common-catalog
)
target_sources(zapata-example-rest-provider
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/provider.cpp
)
target_include_directories(zapata-example-rest-provider
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-engine-transport
    zapata-storage-sqlite
    zapata-storage-connector
    zapata-common-catalog
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-example-rest-provider
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
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
    zapata-engine-transport
    zapata-common-catalog
)
set_target_properties(zapata-example-rest-provider
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata-example-rest-consumer SHARED)
add_dependencies(zapata-example-rest-consumer
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
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
  zapata-parser-http
  zapata-common-catalog
)
target_sources(zapata-example-rest-consumer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/consumer.cpp
)
target_include_directories(zapata-example-rest-consumer
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-engine-transport
    zapata-storage-sqlite
    zapata-storage-connector
    zapata-common-catalog
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-example-rest-consumer
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
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    zapata-parser-http
    zapata-storage-connector
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
    zapata-engine-transport
    zapata-common-catalog
)
set_target_properties(zapata-example-rest-consumer
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(
  TARGETS
    zapata-engine-rest
    zapata-engine-rest-plugin
    zapata-example-rest-provider
    zapata-example-rest-consumer
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_REST_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
