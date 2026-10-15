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
  INTERFACE
    ${SECURITY_KEYSTORE_INCLUDE_DIR}/zapata/keystore/keystore.h
    ${SECURITY_KEYSTORE_INCLUDE_DIR}/zapata/keystore/crypto.h
    ${SECURITY_KEYSTORE_INCLUDE_DIR}/zapata/keystore.h
)
target_include_directories(zapata-security-keystore
  PUBLIC
    ${SECURITY_KEYSTORE_INCLUDE_DIR}
)
target_link_libraries(zapata-security-keystore
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-storage-sqlite
    zapata-storage-connector
    zapata-common-config
    OpenSSL::Crypto
)
set_target_properties(zapata-security-keystore
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
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
    zapata_example_keystore
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY ${SECURITY_KEYSTORE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
