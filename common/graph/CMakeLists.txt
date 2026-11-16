set(COMMON_GRAPH_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-regex-graph SHARED)
add_dependencies(zapata-regex-graph
  zapata-base
)
target_sources(zapata-regex-graph
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graph.cpp
)
target_include_directories(zapata-regex-graph
  PRIVATE
    zapata-base
    ${COMMON_GRAPH_INCLUDE_DIR}
  INTERFACE
    $<BUILD_INTERFACE:${COMMON_GRAPH_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(zapata-regex-graph
  PUBLIC
    zapata-base
)
set_target_properties(zapata-regex-graph
  PROPERTIES
    EXPORT_NAME regex-graph
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-regex-graph.so
)

include(GNUInstallDirs)
install(TARGETS zapata-regex-graph
  EXPORT zapata
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_GRAPH_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
