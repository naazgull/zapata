set(COMMON_BASE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(OpenSSL QUIET)
find_library(MAGIC_LIBRARIES magic)
find_library(UUID_LIBRARIES uuid)

if(NOT (OPENSSL_FOUND AND MAGIC_LIBRARIES AND UUID_LIBRARIES))
  message(STATUS "zapata-base skipped: missing deps (OpenSSL=${OPENSSL_FOUND}, magic=${MAGIC_LIBRARIES}, uuid=${UUID_LIBRARIES})")
  return()
endif()

add_library(zapata-base SHARED)
target_sources(zapata-base
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/spin_mutex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExpectationException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sha512.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/expect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/url.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/html.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sha1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utf8.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NoAttributeNameException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/manip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/convert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base64.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InterruptedException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ParserEOF.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/net.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ClosedException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ascii.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SyntaxErrorException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/send.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CastException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sha256.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NoMoreElementsException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NoSpaceAvailableException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/allocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uuid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/atomic.cpp
  INTERFACE
    ${COMMON_BASE_INCLUDE_DIR}/zapata/crypto.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/mail/manip.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/log/log.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/NoMoreElementsException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/NoSpaceAvailableException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/NoAttributeNameException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/ParserEOF.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/ClosedException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/Exception.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/ExpectationException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/exceptions.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/SyntaxErrorException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/InterruptedException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/CastException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/NoHeaderNameException.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/base/sentry.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/base/expect.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/text/manip.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/text/html.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/text/convert.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/base.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/locks/spin_mutex.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/file/manip.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/net/manip.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/mem/usage.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/crypto/sha512.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/crypto/sha1.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/crypto/sha256.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/atomics/padded_atomic.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/uuid.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/allocator.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/allocator/allocator.h
)
target_include_directories(zapata-base
  PRIVATE
    ${COMMON_BASE_INCLUDE_DIR}
  INTERFACE
    ${COMMON_BASE_INCLUDE_DIR}
)
target_link_libraries(zapata-base
  PUBLIC
    ${MAGIC_LIBRARIES}
    ${UUID_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
)
set_target_properties(zapata-base
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-base.so
)

include(GNUInstallDirs)
install(TARGETS zapata-base
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_BASE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_common_base)
add_dependencies(zapata_example_common_base
  zapata-base
)
target_sources(zapata_example_common_base
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/base.cpp
)
target_include_directories(zapata_example_common_base
  PRIVATE
    zapata-base
)
target_link_libraries(zapata_example_common_base
  PRIVATE
    zapata-base
)
set_target_properties(zapata_example_common_base
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
