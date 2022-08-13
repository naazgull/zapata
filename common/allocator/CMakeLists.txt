set(COMMON_ALLOCATOR_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-allocator INTERFACE)
target_sources(zapata-allocator
  INTERFACE
    ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata/allocator/allocator.h
    ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata/allocator.h
)
target_include_directories(zapata-allocator
  INTERFACE
    ${COMMON_ALLOCATOR_INCLUDE_DIR}
)
target_link_libraries(zapata-allocator
  INTERFACE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
)

include(GNUInstallDirs)
install(TARGETS zapata-allocator
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_ALLOCATOR_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
