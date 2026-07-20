set(BRIDGE_PROLOG_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(SWIPL 10 QUIET)

if(NOT SWIPL_FOUND)
  message(STATUS "zapata-bridge-prolog skipped: SWI-Prolog not found")
  return()
endif()

add_library(zapata-bridge-prolog SHARED)
add_dependencies(zapata-bridge-prolog
  zapata-base
  zapata-parser-json
  zapata-bridge-base
)
target_sources(zapata-bridge-prolog
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prolog.cpp
  INTERFACE
    ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata/prolog.h
    ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata/prolog/helpers.h
    ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata/prolog/prolog.h
)
target_include_directories(zapata-bridge-prolog
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    swipl::libswipl
    ${BRIDGE_PROLOG_INCLUDE_DIR}
  INTERFACE
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_compile_definitions(zapata-bridge-prolog PRIVATE SWICPP=1)
target_link_libraries(zapata-bridge-prolog
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    swipl::libswipl
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
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-bridge-prolog-plugin
  PRIVATE
    zapata-base
    zapata-globals
    zapata-events
    zapata-lockfree
    zapata-parser-json
    zapata-engine-startup
    zapata-bridge-base
    zapata-bridge-prolog
    ${BRIDGE_PROLOG_INCLUDE_DIR}
  INTERFACE
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-prolog-plugin
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-events
    zapata-lockfree
    zapata-engine-startup
    zapata-bridge-base
    zapata-bridge-prolog
    swipl::libswipl
)
set_target_properties(zapata-bridge-prolog-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

add_library(zapata_bridge_prolog_bindings SHARED)
add_dependencies(zapata_bridge_prolog_bindings
  zapata-base
  zapata-parser-json
  zapata-events
  zapata-engine-startup
  zapata-bridge-base
  zapata-bridge-prolog
)
target_sources(zapata_bridge_prolog_bindings
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bindings.cpp
)
target_include_directories(zapata_bridge_prolog_bindings
  PRIVATE
    zapata-base
    zapata-common-catalog
    zapata-globals
    zapata-events
    zapata-lockfree
    zapata-parser-json
    zapata-engine-startup
    zapata-bridge-base
    zapata-bridge-prolog
    zapata-engine-rest
    zapata-engine-transport
    ${BRIDGE_PROLOG_INCLUDE_DIR}
  INTERFACE
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata_bridge_prolog_bindings
  PUBLIC
    zapata-base
    zapata-common-catalog
    zapata-parser-json
    zapata-events
    zapata-lockfree
    zapata-engine-startup
    zapata-bridge-base
    zapata-bridge-prolog
    zapata-engine-rest
    zapata-engine-transport
    swipl::libswipl
    ${SQLite3_LIBRARIES}
    zapata-storage-sqlite
)
set_target_properties(zapata_bridge_prolog_bindings
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-bridge-prolog zapata-bridge-prolog-plugin zapata_bridge_prolog_bindings
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_bridge_prolog)
add_dependencies(zapata_example_bridge_prolog
  zapata-base
  zapata-parser-json
  zapata-bridge-base
  zapata-bridge-prolog
)
target_sources(zapata_example_bridge_prolog
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/prolog_bridge.cpp
)
target_include_directories(zapata_example_bridge_prolog
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-prolog
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata_example_bridge_prolog
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-prolog
    swipl::libswipl
)
set_target_properties(zapata_example_bridge_prolog
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_executable(zapata_example_integration_prolog)
add_dependencies(zapata_example_integration_prolog
  zapata-base
  zapata-parser-json
  zapata-bridge-base
  zapata-bridge-prolog
)
target_sources(zapata_example_integration_prolog
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/prolog_integration.cpp
)
target_include_directories(zapata_example_integration_prolog
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-prolog
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata_example_integration_prolog
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-prolog
    swipl::libswipl
)
set_target_properties(zapata_example_integration_prolog
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
