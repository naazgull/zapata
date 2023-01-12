set(IO_SOCKET_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-io-socket SHARED)
add_dependencies(zapata-io-socket
  zapata-base
  zapata-io-stream
)
target_sources(zapata-io-socket
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/socket_stream.cpp
  INTERFACE
    ${IO_SOCKET_INCLUDE_DIR}/zapata/net/socket/socket_stream.h
    ${IO_SOCKET_INCLUDE_DIR}/zapata/net/socket.h
)
target_include_directories(zapata-io-socket
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    ${IO_SOCKET_INCLUDE_DIR}
  INTERFACE
    ${IO_SOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata-io-socket
  PRIVATE
    ssl
    crypto
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
)
set_target_properties(zapata-io-socket
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-io-socket.so
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
add_dependencies(zapata_test_io_socket_client
  zapata-base
  zapata-io-stream
  zapata-io-socket
)
target_sources(zapata_test_io_socket_client
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_client.cpp
)
target_include_directories(zapata_test_io_socket_client
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    ${IO_SOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata_test_io_socket_client
  PRIVATE
    ssl
    crypto
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-socket,LIBRARIES>
)
set_target_properties(zapata_test_io_socket_client
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_executable(zapata_test_io_socket_server)
add_dependencies(zapata_test_io_socket_server
  zapata-base
  zapata-io-stream
  zapata-io-socket
)
target_sources(zapata_test_io_socket_server
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_server.cpp
)
target_include_directories(zapata_test_io_socket_server
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    ${IO_SOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata_test_io_socket_server
  PRIVATE
    ssl
    crypto
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-socket,LIBRARIES>
)
set_target_properties(zapata_test_io_socket_server
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
