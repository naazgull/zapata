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
  INTERFACE
    ${COMMON_GLOBALS_INCLUDE_DIR}/zapata/globals/thread_local_variable.h
    ${COMMON_GLOBALS_INCLUDE_DIR}/zapata/globals/cached.h
    ${COMMON_GLOBALS_INCLUDE_DIR}/zapata/globals/globals.h
    ${COMMON_GLOBALS_INCLUDE_DIR}/zapata/globals.h
)
target_include_directories(zapata-globals
  PRIVATE
    zapata-base
    ${COMMON_GLOBALS_INCLUDE_DIR}
  INTERFACE
    ${COMMON_GLOBALS_INCLUDE_DIR}
)
target_link_libraries(zapata-globals
  PRIVATE
    zapata-base
)
set_target_properties(zapata-globals
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-globals.so
)

include(GNUInstallDirs)
install(TARGETS zapata-globals
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_GLOBALS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_common_globals)
add_dependencies(zapata_test_common_globals
  zapata-base
  zapata-globals
)
target_sources(zapata_test_common_globals
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_globals
  PRIVATE
    zapata-base
    ${COMMON_GLOBALS_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_globals
  PRIVATE
    zapata-base
    zapata-globals
)

set_target_properties(zapata_test_common_globals
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
