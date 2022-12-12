set(BRIDGE_BASE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-bridge-base SHARED)
add_dependencies(zapata-bridge-base
  zapata-base
  zapata-parser-json
)
target_sources(zapata-bridge-base
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bridge.cpp
    ${BRIDGE_BASE_INCLUDE_DIR}/zapata/bridge/bridge.h
    ${BRIDGE_BASE_INCLUDE_DIR}/zapata/bridge.h
)
target_include_directories(zapata-bridge-base
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_BASE_INCLUDE_DIR}
  INTERFACE
    ${BRIDGE_BASE_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-base
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
)
set_target_properties(zapata-bridge-base
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-bridge-base.so
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
add_dependencies(zapata_test_bridge_base
  zapata-base
  zapata-parser-json
  zapata-bridge-base
)
target_sources(zapata_test_bridge_base
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_bridge_base
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_BASE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_bridge_base
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
)
set_target_properties(zapata_test_bridge_base
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
