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
  INTERFACE
    ${COMMON_GRAPH_INCLUDE_DIR}/zapata/graph.h
    ${COMMON_GRAPH_INCLUDE_DIR}/zapata/graph/node.h
)
target_include_directories(zapata-regex-graph
  PRIVATE
    zapata-base
    ${COMMON_GRAPH_INCLUDE_DIR}
  INTERFACE
    ${COMMON_GRAPH_INCLUDE_DIR}
)
target_link_libraries(zapata-regex-graph
  PRIVATE
    zapata-base
)  
set_target_properties(zapata-regex-graph
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-regex-graph.so
)

include(GNUInstallDirs)
install(TARGETS zapata-regex-graph
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_GRAPH_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
