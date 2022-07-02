set(IO_STREAM_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-io-stream SHARED)

target_sources(zapata-io-stream
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/streams.cpp
  INTERFACE
    ${IO_STREAM_INCLUDE_DIR}/zapata/streams/streams.h
    ${IO_STREAM_INCLUDE_DIR}/zapata/streams.h
)
target_include_directories(zapata-io-stream
  INTERFACE
    ${IO_STREAM_INCLUDE_DIR}
)
target_link_libraries(zapata-io-stream
  INTERFACE
    systemd
    zapata-base
)

set_target_properties(zapata-io-stream
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)

include(GNUInstallDirs)
install(TARGETS zapata-io-stream
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${IO_STREAM_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_io_stream)

target_sources(zapata_test_io_stream
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_io_stream
  PUBLIC
    ${IO_STREAM_INCLUDE_DIR}
)
target_link_libraries(zapata_test_io_stream
  PUBLIC
    systemd
    zapata-base
    zapata-io-stream
)

set_target_properties(zapata_test_io_stream
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)
