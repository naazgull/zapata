set(BRIDGE_PROLOG_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-bridge-prolog SHARED)
add_dependencies(zapata-bridge-prolog
  zapata-base
  zapata-parser-json
  zapata-bridge-base
)
target_sources(zapata-bridge-prolog
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prolog.cpp
    ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata/prolog.h
    ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata/prolog/prolog.h
)
target_include_directories(zapata-bridge-prolog
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_PROLOG_INCLUDE_DIR}
  INTERFACE    
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-prolog
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
)
set_target_properties(zapata-bridge-prolog
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-bridge-prolog.so
)

add_library(zapata-bridge-prolog-plugin SHARED)
add_dependencies(zapata-bridge-prolog-plugin
  zapata-base
  zapata-parser-json
  zapata-events
  zapata-engine-startup
  zapata-bridge-base
  zapata-bridge-prolog
)
target_sources(zapata-bridge-prolog-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-bridge-prolog-plugin
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-events,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-engine-startup,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-prolog,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_PROLOG_INCLUDE_DIR}
  INTERFACE
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-prolog-plugin
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
    $<TARGET_PROPERTY:zapata-lockfree,LIBRARIES>
    $<TARGET_PROPERTY:zapata-engine-startup,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-prolog,LIBRARIES>
)
set_target_properties(zapata-bridge-prolog-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-bridge-prolog zapata-bridge-prolog-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_bridge_prolog)
add_dependencies(zapata_test_bridge_prolog
  zapata-base
  zapata-parser-json
  zapata-bridge-base
  zapata-bridge-prolog
)
target_sources(zapata_test_bridge_prolog
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_bridge_prolog
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-prolog,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata_test_bridge_prolog
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-prolog,LIBRARIES>
)
set_target_properties(zapata_test_bridge_prolog
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
