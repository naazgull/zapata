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
)
target_include_directories(zapata-io-pipe
  PRIVATE
    zapata-base
    zapata-io-stream
    ${IO_PIPE_INCLUDE_DIR}
  INTERFACE
    $<BUILD_INTERFACE:${IO_PIPE_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(zapata-io-pipe
  PUBLIC
    zapata-base
    zapata-io-stream
)
set_target_properties(zapata-io-pipe
  PROPERTIES
    EXPORT_NAME io-pipe
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-io-pipe.so
)

include(GNUInstallDirs)
install(TARGETS zapata-io-pipe
  EXPORT zapata
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${IO_PIPE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_io_pipe)
add_dependencies(zapata_example_io_pipe
  zapata-base
  zapata-io-stream
  zapata-io-pipe
)
target_sources(zapata_example_io_pipe
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/pipe.cpp
)
target_include_directories(zapata_example_io_pipe
  PRIVATE
    zapata-base
    zapata-io-stream
    ${IO_PIPE_INCLUDE_DIR}
)
target_link_libraries(zapata_example_io_pipe
  PRIVATE
    zapata-base
    zapata-io-stream
    zapata-io-pipe
)
set_target_properties(zapata_example_io_pipe
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
