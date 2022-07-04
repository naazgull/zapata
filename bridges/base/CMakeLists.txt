set(BRIDGE_BASE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-bridge-base SHARED)

target_sources(zapata-bridge-base
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bridge.cpp
  INTERFACE
    ${BRIDGE_BASE_INCLUDE_DIR}/zapata/bridge/bridge.h
    ${BRIDGE_BASE_INCLUDE_DIR}/zapata/bridge.h
)
target_include_directories(zapata-bridge-base
  INTERFACE
    ${BRIDGE_BASE_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-base
  INTERFACE
    zapata-base
    zapata-parser-json
)

set_target_properties(zapata-bridge-base
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-bridge-base
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${BRIDGE_BASE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_bridge_base)

target_sources(zapata_test_bridge_base
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_bridge_base
  PUBLIC
    ${BRIDGE_BASE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_bridge_base
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-bridge-base
)

set_target_properties(zapata_test_bridge_base
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
