set(COMMON_BASE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(libzapata-base SHARED)

target_sources(libzapata-base
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/spin_lock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ExpectationException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/sha512.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/expect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/Exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/url.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/html.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/usage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/sha1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/utf8.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/NoAttributeNameException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/manip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/convert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/performative.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/base64.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/InterruptedException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ParserEOF.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/net.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ClosedException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ascii.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/SyntaxErrorException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/send.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/CastException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/sha256.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/NoMoreElementsException.cpp    
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
    ${COMMON_BASE_INCLUDE_DIR}/zapata/base/unconfig.h
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
)
target_include_directories(libzapata-base
  PUBLIC
    ${COMMON_BASE_INCLUDE_DIR}
)
target_link_libraries(libzapata-base
  PUBLIC
    magic ossp-uuid++ crypto++
)
  
set_target_properties(libzapata-base
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

include(GNUInstallDirs)
install(TARGETS libzapata-base
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_BASE_INCLUDE_DIR}/hermes
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
