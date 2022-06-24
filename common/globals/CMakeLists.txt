set(COMMON_GLOBALS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(libzapata-globals INTERFACE)

target_sources(libzapata-globals
  INTERFACE
    ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata/globals/thread_local_variable.h
    ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata/globals/cached.h
    ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata/globals/globals.h
    ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata/globals.h
)
target_include_directories(libzapata-globals
  INTERFACE
    ${COMMON_GLOBALS_INCLUDE_DIR}
)
target_link_libraries(libzapata-globals
  INTERFACE
    $<TARGET_OBJECTS:libzapata-base>
)

include(GNUInstallDirs)
install(TARGETS libzapata-globals
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_GLOBALS_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zglobals)

target_sources(zglobals
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/globals/main.cpp
)
target_include_directories(zglobals
  PUBLIC
    ${JOBS_INCLUDE_DIR}
)
target_link_libraries(zglobals
  PUBLIC
    magic ossp-uuid++ crypto++ $<TARGET_OBJECTS:libzapata-base>
)

set_target_properties(zglobals
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)
