set(NETWORK_TCP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Proton QUIET)

if(NOT Proton_FOUND)
  message(STATUS "zapata-net-amqp skipped: qpid-proton library not found")
  return()
endif()

add_library(zapata-net-amqp SHARED)
add_dependencies(zapata-net-amqp
  zapata-base
  zapata-common-catalog
  zapata-engine-startup
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-net-self
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-storage-sqlite
)
target_sources(zapata-net-amqp
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/amqp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/amqp_stream.cpp
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/amqp.h
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/transport/amqp.h
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/transport/amqp_stream.h
)
target_include_directories(zapata-net-amqp
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-self
    zapata-net-transport
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${NETWORK_TCP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-amqp
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-self
    zapata-net-transport
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    Proton::qpid-proton
)
set_target_properties(zapata-net-amqp
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-amqp.so
)

add_library(zapata-net-amqp-plugin SHARED)
add_dependencies(zapata-net-amqp-plugin
  zapata-base
  zapata-common-catalog
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-amqp
  zapata-net-self
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-runtime
)
target_sources(zapata-net-amqp-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-amqp-plugin
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-amqp
    zapata-net-self
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-runtime
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${NETWORK_TCP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-amqp-plugin
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-net-amqp
    zapata-net-self
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-runtime
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    Proton::qpid-proton
 )
set_target_properties(zapata-net-amqp-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata-example-amqp-consumer SHARED)
add_dependencies(zapata-example-amqp-consumer
  zapata-base
  zapata-common-catalog
  zapata-engine-startup
  zapata-engine-transport
  zapata-engine-rest
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-amqp
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-example-amqp-consumer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/consumer.cpp
)
target_include_directories(zapata-example-amqp-consumer
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-engine-transport
    zapata-engine-rest
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-amqp
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-example-amqp-consumer
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-engine-transport
    zapata-engine-rest
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-amqp
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
)
set_target_properties(zapata-example-amqp-consumer
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata-example-amqp-producer SHARED)
add_dependencies(zapata-example-amqp-producer
  zapata-base
  zapata-common-catalog
  zapata-engine-startup
  zapata-engine-transport
  zapata-engine-rest
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-amqp
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-example-amqp-producer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/producer.cpp
)
target_include_directories(zapata-example-amqp-producer
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-engine-transport
    zapata-engine-rest
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-amqp
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-example-amqp-producer
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-engine-transport
    zapata-engine-rest
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-amqp
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
)
set_target_properties(zapata-example-amqp-producer
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS
    zapata-net-amqp
    zapata-net-amqp-plugin
    zapata-example-amqp-consumer
    zapata-example-amqp-producer
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_TCP_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_amqp_transport)
add_dependencies(zapata_example_amqp_transport
  zapata-base
  zapata-common-catalog
  zapata-parser-json
  zapata-parser-uri
  zapata-net-amqp
  zapata-net-self
  zapata-net-transport
)
target_sources(zapata_example_amqp_transport
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/amqp_transport.cpp
)
target_include_directories(zapata_example_amqp_transport
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-amqp
    zapata-net-self
    zapata-net-transport
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata_example_amqp_transport
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-amqp
    zapata-net-self
    zapata-net-transport
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    Proton::qpid-proton
)
set_target_properties(zapata_example_amqp_transport
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
