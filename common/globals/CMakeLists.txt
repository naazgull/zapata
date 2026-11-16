set(COMMON_GLOBALS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-globals SHARED)
add_dependencies(zapata-globals
  zapata-base
)
target_sources(zapata-globals
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/globals.cpp
)
target_include_directories(zapata-globals
  PRIVATE
    zapata-base
    ${COMMON_GLOBALS_INCLUDE_DIR}
  INTERFACE
    $<BUILD_INTERFACE:${COMMON_GLOBALS_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(zapata-globals
  PUBLIC
    zapata-base
)
set_target_properties(zapata-globals
  PROPERTIES
    EXPORT_NAME globals
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-globals.so
)

include(GNUInstallDirs)
install(TARGETS zapata-globals
  EXPORT zapata
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_GLOBALS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_common_globals)
add_dependencies(zapata_example_common_globals
  zapata-base
  zapata-globals
)
target_sources(zapata_example_common_globals
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/globals.cpp
)
target_include_directories(zapata_example_common_globals
  PRIVATE
    zapata-base
    ${COMMON_GLOBALS_INCLUDE_DIR}
)
target_link_libraries(zapata_example_common_globals
  PRIVATE
    zapata-base
    zapata-globals
)

set_target_properties(zapata_example_common_globals
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
