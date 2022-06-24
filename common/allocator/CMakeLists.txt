set(COMMON_ALLOCATOR_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(libzapata-allocator INTERFACE)

target_sources(libzapata-allocator
  INTERFACE
    ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata/allocator/allocator.h
    ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata/allocator.h
)
target_include_directories(libzapata-allocator
  INTERFACE
    ${COMMON_ALLOCATOR_INCLUDE_DIR}
)
target_link_libraries(libzapata-allocator
  INTERFACE
    $<TARGET_OBJECTS:libzapata-base>
)

include(GNUInstallDirs)
install(TARGETS libzapata-allocator
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
