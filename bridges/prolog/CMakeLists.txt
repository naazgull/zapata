set(BRIDGE_PROLOG_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-bridge-prolog SHARED)

target_sources(zapata-bridge-prolog
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/prolog.cpp
    ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata/prolog.h
    ${BRIDGE_PROLOG_INCLUDE_DIR}/zapata/prolog/prolog.h
)
target_include_directories(zapata-bridge-prolog
  PUBLIC
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-prolog
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-bridge-base
)

set_target_properties(zapata-bridge-prolog
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-bridge-prolog-plugin SHARED)

target_sources(zapata-bridge-prolog-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-bridge-prolog-plugin
  PUBLIC
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-prolog-plugin
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-prolog
)

set_target_properties(zapata-bridge-prolog-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
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

target_sources(zapata_test_bridge_prolog
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_bridge_prolog
  PRIVATE
    ${BRIDGE_PROLOG_INCLUDE_DIR}
)
target_link_libraries(zapata_test_bridge_prolog
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-prolog
)

set_target_properties(zapata_test_bridge_prolog
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
