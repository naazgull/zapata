set(IO_STREAM_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_library(SYSTEMD_LIBRARIES systemd)

if(NOT SYSTEMD_LIBRARIES)
  message(STATUS "zapata-io-stream skipped: libsystemd not found")
  return()
endif()

add_library(zapata-io-stream SHARED)
add_dependencies(zapata-io-stream
  zapata-base
)
target_sources(zapata-io-stream
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/streams.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/event_stream.cpp
  INTERFACE
    ${IO_STREAM_INCLUDE_DIR}/zapata/streams/streams.h
    ${IO_STREAM_INCLUDE_DIR}/zapata/streams/event_stream.h
    ${IO_STREAM_INCLUDE_DIR}/zapata/streams.h
)
target_include_directories(zapata-io-stream
  PRIVATE
    zapata-base
    ${IO_STREAM_INCLUDE_DIR}
  INTERFACE
    ${IO_STREAM_INCLUDE_DIR}
)
target_link_libraries(zapata-io-stream
  PRIVATE
    ${SYSTEMD_LIBRARIES}
    zapata-base
)
set_target_properties(zapata-io-stream
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-io-stream.so
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

add_executable(zapata_example_io_stream)
add_dependencies(zapata_example_io_stream
  zapata-base
  zapata-io-stream
)
target_sources(zapata_example_io_stream
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/stream.cpp
)
target_include_directories(zapata_example_io_stream
  PRIVATE
    zapata-base
    ${IO_STREAM_INCLUDE_DIR}
)
target_link_libraries(zapata_example_io_stream
  PRIVATE
    ${SYSTEMD_LIBRARIES}
    zapata-base
    zapata-io-stream
)
set_target_properties(zapata_example_io_stream
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
