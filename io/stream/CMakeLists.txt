set(IO_STREAM_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-io-stream SHARED)
add_dependencies(zapata-io-stream
  zapata-base
)
target_sources(zapata-io-stream
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/streams.cpp
    ${IO_STREAM_INCLUDE_DIR}/zapata/streams/streams.h
    ${IO_STREAM_INCLUDE_DIR}/zapata/streams.h
)
target_include_directories(zapata-io-stream
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${IO_STREAM_INCLUDE_DIR}
  INTERFACE
    ${IO_STREAM_INCLUDE_DIR}
)
target_link_libraries(zapata-io-stream
  PRIVATE
    systemd
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
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

add_executable(zapata_test_io_stream)
add_dependencies(zapata_test_io_stream
  zapata-base
  zapata-io-stream
)
target_sources(zapata_test_io_stream
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_io_stream
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${IO_STREAM_INCLUDE_DIR}
)
target_link_libraries(zapata_test_io_stream
  PRIVATE
    systemd
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
)
set_target_properties(zapata_test_io_stream
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
