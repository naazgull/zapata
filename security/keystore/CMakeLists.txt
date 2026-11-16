set(SECURITY_KEYSTORE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(OpenSSL QUIET)

if(NOT OPENSSL_FOUND)
  message(STATUS "zapata-security-keystore skipped: OpenSSL not found")
  return()
endif()

add_library(zapata-security-keystore SHARED)
add_dependencies(zapata-security-keystore
  zapata-base
  zapata-parser-json
  zapata-storage-sqlite
  zapata-storage-connector
  zapata-common-config
)
target_sources(zapata-security-keystore
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/keystore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/keys_store.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key_document.cpp
)
target_include_directories(zapata-security-keystore
  PRIVATE
    ${SECURITY_KEYSTORE_INCLUDE_DIR}
  INTERFACE
    $<BUILD_INTERFACE:${SECURITY_KEYSTORE_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(zapata-security-keystore
  PUBLIC
    zapata-base
    zapata-common-catalog
    zapata-common-config
    zapata-engine-rest
    zapata-engine-transport
    zapata-events
    zapata-lockfree
    zapata-parser-json
    zapata-storage-sqlite
    zapata-storage-connector
    OpenSSL::Crypto
)
set_target_properties(zapata-security-keystore
  PROPERTIES
    EXPORT_NAME security-keystore
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)


add_library(zapata-security-keystore-plugin SHARED)
target_sources(zapata-security-keystore-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-security-keystore-plugin
  PUBLIC
    ${SECURITY_KEYSTORE_INCLUDE_DIR}
)
target_link_libraries(zapata-security-keystore-plugin
  PUBLIC
    zapata-base
    zapata-common-catalog
    zapata-common-config
    zapata-engine-rest
    zapata-engine-transport
    zapata-events
    zapata-lockfree
    zapata-parser-json
    zapata-parser-http
    zapata-security-keystore
    zapata-storage-sqlite
    zapata-storage-connector
    OpenSSL::Crypto
)
set_target_properties(zapata-security-keystore-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_executable(zapata_example_keystore)
add_dependencies(zapata_example_keystore
  zapata-security-keystore
)
target_sources(zapata_example_keystore
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/keystore.cpp
)
target_include_directories(zapata_example_keystore
  PRIVATE
    ${SECURITY_KEYSTORE_INCLUDE_DIR}
)
target_link_libraries(zapata_example_keystore
  PRIVATE
    zapata-security-keystore
)
set_target_properties(zapata_example_keystore
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(
  TARGETS
    zapata-security-keystore
  EXPORT zapata
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
  TARGETS
    zapata-security-keystore-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${SECURITY_KEYSTORE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
