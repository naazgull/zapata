set(SECURITY_OAUTH2_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-security-oauth2 SHARED)

target_sources(zapata-security-oauth2
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/oauth2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/authorize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/refresh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/validate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/device_authorization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/approve.cpp
  INTERFACE
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/oauth2.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/services/authorize.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/services/token.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/services/refresh.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/services/validate.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/services/device_authorization.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/services/approve.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2.h
)
target_include_directories(zapata-security-oauth2
  PUBLIC
    ${SECURITY_OAUTH2_INCLUDE_DIR}
)
target_link_libraries(zapata-security-oauth2
  PUBLIC
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
    zapata-common-config
    zapata-engine-rest
)

set_target_properties(zapata-security-oauth2
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-security-oauth2-plugin SHARED)

target_sources(zapata-security-oauth2-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-security-oauth2-plugin
  PUBLIC
    ${SECURITY_OAUTH2_INCLUDE_DIR}
)
target_link_libraries(zapata-security-oauth2-plugin
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-http
    zapata-engine-rest
    zapata-security-oauth2
)

set_target_properties(zapata-security-oauth2-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-example-oauth2-provider SHARED)

target_sources(zapata-example-oauth2-provider
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/provider.cpp
)
target_link_libraries(zapata-example-oauth2-provider
  PRIVATE
    zapata-security-oauth2
)

set_target_properties(zapata-example-oauth2-provider
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-example-oauth2-consumer SHARED)

target_sources(zapata-example-oauth2-consumer
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/consumer.cpp
)
target_link_libraries(zapata-example-oauth2-consumer
  PRIVATE
    zapata-security-oauth2
)

set_target_properties(zapata-example-oauth2-consumer
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(
  TARGETS
    zapata-security-oauth2
    zapata-security-oauth2-plugin
    zapata-example-oauth2-provider
    zapata-example-oauth2-consumer
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
