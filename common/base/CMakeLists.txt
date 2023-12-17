set(COMMON_BASE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-base SHARED)
target_sources(zapata-base
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/spin_lock.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/performative.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/allocator.cpp    
  INTERFACE
    ${COMMON_BASE_INCLUDE_DIR}/zapata/mail/manip.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/log/log.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/exceptions/NoMoreElementsException.h
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
    ${COMMON_BASE_INCLUDE_DIR}/zapata/base/performative.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/base/expect.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/text/manip.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/text/html.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/text/convert.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/base.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/locks/spin_lock.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/file/manip.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/net/manip.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/mem/ref_ptr.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/mem/usage.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/crypto/sha512.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/crypto/sha1.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/crypto/sha256.h
    ${COMMON_BASE_INCLUDE_DIR}/zapata/atomics/padded_atomic.h
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
    magic
    ossp-uuid++
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

add_executable(zapata_test_common_base)
add_dependencies(zapata_test_common_base
  zapata-base
)
target_sources(zapata_test_common_base
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_base
  PRIVATE
    zapata-base
)
target_link_libraries(zapata_test_common_base
  PRIVATE
    zapata-base
)
set_target_properties(zapata_test_common_base
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
