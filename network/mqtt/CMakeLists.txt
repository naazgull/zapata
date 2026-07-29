set(NETWORK_TCP_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (NOT MOSQUITTO_INCLUDE_DIR)
  find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h)
endif()

if (NOT MOSQUITTO_LIBRARY)
  find_library(
    MOSQUITTO_LIBRARY
    NAMES mosquitto)
endif()

if(NOT MOSQUITTO_LIBRARY OR NOT MOSQUITTO_INCLUDE_DIR)
  message(STATUS "zapata-net-mqtt skipped: mosquitto library not found")
  return()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  MOSQUITTO DEFAULT_MSG
  MOSQUITTO_LIBRARY MOSQUITTO_INCLUDE_DIR)

set(MOSQUITTO_LIBRARIES ${MOSQUITTO_LIBRARY})
mark_as_advanced(MOSQUITTO_INCLUDE_DIR MOSQUITTO_LIBRARY)

add_library(zapata-net-mqtt SHARED)
add_dependencies(zapata-net-mqtt
  zapata-base
  zapata-common-catalog
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-net-self
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
  zapata-storage-sqlite
)
target_sources(zapata-net-mqtt
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mqtt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mqtt_stream.cpp
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/mqtt.h
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/transport/mqtt.h
    ${NETWORK_TCP_INCLUDE_DIR}/zapata/net/transport/mqtt_stream.h
)
target_include_directories(zapata-net-mqtt

  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-self
    zapata-net-transport
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${MOSQUITTO_INCLUDE_DIR}
    ${NETWORK_TCP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-mqtt
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-self
    zapata-net-transport
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${MOSQUITTO_LIBRARY}
)
set_target_properties(zapata-net-mqtt
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-mqtt.so
)

add_library(zapata-net-mqtt-plugin SHARED)
add_dependencies(zapata-net-mqtt-plugin
  zapata-base
  zapata-common-catalog
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-mqtt
  zapata-net-self
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-mqtt-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-mqtt-plugin
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-lockfree
    zapata-net-mqtt
    zapata-net-self
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${MOSQUITTO_INCLUDE_DIR}
    ${NETWORK_TCP_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata-net-mqtt-plugin
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-engine-startup
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-io-socket
    zapata-net-mqtt
    zapata-net-self
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${MOSQUITTO_LIBRARY}
 )
set_target_properties(zapata-net-mqtt-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata-example-mqtt-consumer SHARED)
add_dependencies(zapata-example-mqtt-consumer
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
  zapata-net-mqtt
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-example-mqtt-consumer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/consumer.cpp
)
target_include_directories(zapata-example-mqtt-consumer
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
    zapata-net-mqtt
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-example-mqtt-consumer
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
    zapata-net-mqtt
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
)
set_target_properties(zapata-example-mqtt-consumer
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata-example-mqtt-producer SHARED)
add_dependencies(zapata-example-mqtt-producer
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
  zapata-net-mqtt
  zapata-net-transport
  zapata-ontology
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-example-mqtt-producer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/producer.cpp
)
target_include_directories(zapata-example-mqtt-producer
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
    zapata-net-mqtt
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    ${ENGINE_REST_INCLUDE_DIR}
)
target_link_libraries(zapata-example-mqtt-producer
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
    zapata-net-mqtt
    zapata-net-transport
    zapata-ontology
    zapata-parser-json
    zapata-parser-uri
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
)
set_target_properties(zapata-example-mqtt-producer
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS
    zapata-net-mqtt
    zapata-net-mqtt-plugin
    zapata-example-mqtt-consumer
    zapata-example-mqtt-producer
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_TCP_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_mqtt_transport)
add_dependencies(zapata_example_mqtt_transport
  zapata-base
  zapata-common-catalog
  zapata-parser-json
  zapata-parser-uri
  zapata-net-mqtt
  zapata-net-self
  zapata-net-transport
)
target_sources(zapata_example_mqtt_transport
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/mqtt_transport.cpp
)
target_include_directories(zapata_example_mqtt_transport
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-mqtt
    zapata-net-self
    zapata-net-transport
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${MOSQUITTO_INCLUDE_DIR}
    ${NETWORK_TCP_INCLUDE_DIR}
)
target_link_libraries(zapata_example_mqtt_transport
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-io-stream
    zapata-io-socket
    zapata-parser-json
    zapata-parser-uri
    zapata-net-mqtt
    zapata-net-self
    zapata-net-transport
    zapata-storage-sqlite
    ${SQLite3_LIBRARIES}
    ${MOSQUITTO_LIBRARY}
)
set_target_properties(zapata_example_mqtt_transport
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
