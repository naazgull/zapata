set(COMMON_LOCKFREE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-lockfree INTERFACE)
add_dependencies(zapata-lockfree
  zapata-base
  zapata-globals
)
target_sources(zapata-lockfree
  INTERFACE
    ${COMMON_LOCKFREE_INCLUDE_DIR}/zapata/lockfree.h
    ${COMMON_LOCKFREE_INCLUDE_DIR}/zapata/lockfree/hazard_ptr.h
    ${COMMON_LOCKFREE_INCLUDE_DIR}/zapata/lockfree/queue.h
)
target_include_directories(zapata-lockfree
  INTERFACE
    zapata-base
    zapata-globals
    ${COMMON_LOCKFREE_INCLUDE_DIR}
)
target_link_libraries(zapata-lockfree
  INTERFACE
    zapata-base
    zapata-globals
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
add_dependencies(zapata_test_common_lockfree
  zapata-base
  zapata-globals
  zapata-lockfree
)
target_sources(zapata_test_common_lockfree
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_lockfree
  PRIVATE
    zapata-base
    zapata-globals
    ${COMMON_LOCKFREE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_lockfree
  PRIVATE
    zapata-base
    zapata-globals
)
set_target_properties(zapata_test_common_lockfree
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
