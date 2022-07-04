set(IO_SOCKET_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-io-socket SHARED)

target_sources(zapata-io-socket
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/socket_stream.cpp
  INTERFACE
    ${IO_SOCKET_INCLUDE_DIR}/zapata/net/socket/socket_stream.h
    ${IO_SOCKET_INCLUDE_DIR}/zapata/net/socket.h
)
target_include_directories(zapata-io-socket
  INTERFACE
    ${IO_SOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata-io-socket
  INTERFACE
    ssl
    crypto
    zapata-base
    zapata-io-stream
)

set_target_properties(zapata-io-socket
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-io-socket
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${IO_SOCKET_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_io_socket_client)

target_sources(zapata_test_io_socket_client
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_client.cpp
)
target_include_directories(zapata_test_io_socket_client
  PUBLIC
    ${IO_SOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata_test_io_socket_client
  PUBLIC
    ssl
    crypto
    zapata-base
    zapata-io-stream
    zapata-io-socket
)

set_target_properties(zapata_test_io_socket_client
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_executable(zapata_test_io_socket_server)

target_sources(zapata_test_io_socket_server
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_server.cpp
)
target_include_directories(zapata_test_io_socket_server
  PUBLIC
    ${IO_SOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata_test_io_socket_server
  PUBLIC
    ssl
    crypto
    zapata-base
    zapata-io-stream
    zapata-io-socket
)

set_target_properties(zapata_test_io_socket_server
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
