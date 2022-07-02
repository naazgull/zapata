set(COMMON_LOCKFREE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-lockfree INTERFACE)

target_sources(zapata-lockfree
  INTERFACE
    ${COMMON_LOCKFREE_INCLUDE_DIR}/zapata/lockfree.h
    ${COMMON_LOCKFREE_INCLUDE_DIR}/zapata/lockfree/hazard_ptr.h
    ${COMMON_LOCKFREE_INCLUDE_DIR}/zapata/lockfree/queue.h
)
target_include_directories(zapata-lockfree
  INTERFACE
    ${COMMON_LOCKFREE_INCLUDE_DIR}
)
target_link_libraries(zapata-lockfree
  INTERFACE
    zapata-base
)

include(GNUInstallDirs)
install(TARGETS zapata-lockfree
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_LOCKFREE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_common_lockfree)

target_sources(zapata_test_common_lockfree
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_lockfree
  PUBLIC
    ${COMMON_LOCKFREE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_lockfree
  PUBLIC
    zapata-base
)

set_target_properties(zapata_test_common_lockfree
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fno-omit-frame-pointer"
    LINK_FLAGS ""
)