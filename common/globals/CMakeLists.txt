set(COMMON_GLOBALS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-globals INTERFACE)

target_sources(zapata-globals
  INTERFACE
    ${COMMON_GLOBALS_INCLUDE_DIR}/zapata/globals/thread_local_variable.h
    ${COMMON_GLOBALS_INCLUDE_DIR}/zapata/globals/cached.h
    ${COMMON_GLOBALS_INCLUDE_DIR}/zapata/globals/globals.h
    ${COMMON_GLOBALS_INCLUDE_DIR}/zapata/globals.h
)
target_include_directories(zapata-globals
  INTERFACE
    ${COMMON_GLOBALS_INCLUDE_DIR}
)
target_link_libraries(zapata-globals
  INTERFACE
    zapata-base
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

target_sources(zapata_test_common_globals
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_globals
  PUBLIC
    ${COMMON_GLOBALS_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_globals
  PUBLIC
    zapata-base
)

set_target_properties(zapata_test_common_globals
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)
