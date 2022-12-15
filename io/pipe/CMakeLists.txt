set(IO_PIPE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-io-pipe SHARED)
add_dependencies(zapata-io-pipe
  zapata-base
  zapata-io-stream
)
target_sources(zapata-io-pipe
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipe_stream.cpp
  INTERFACE
    ${IO_PIPE_INCLUDE_DIR}/zapata/io/pipe/pipe_stream.h
    ${IO_PIPE_INCLUDE_DIR}/zapata/io/pipe.h
)
target_include_directories(zapata-io-pipe
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    ${IO_PIPE_INCLUDE_DIR}
  INTERFACE
    ${IO_PIPE_INCLUDE_DIR}
)
target_link_libraries(zapata-io-pipe
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
)
set_target_properties(zapata-io-pipe
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-io-pipe.so
)

include(GNUInstallDirs)
install(TARGETS zapata-io-pipe
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_EVENTS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_io_pipe)
add_dependencies(zapata_test_io_pipe
  zapata-base
  zapata-io-stream
  zapata-io-pipe
)
target_sources(zapata_test_io_pipe
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_io_pipe
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    ${IO_PIPE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_io_pipe
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-pipe,LIBRARIES>
)
set_target_properties(zapata_test_io_pipe
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
