set(ENGINE_AUTOMATON_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-engine-automaton SHARED)

target_sources(zapata-engine-automaton
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/automaton.cpp
    ${ENGINE_AUTOMATON_INCLUDE_DIR}/zapata/automaton.h
    ${ENGINE_AUTOMATON_INCLUDE_DIR}/zapata/automaton/automaton.h
)
target_include_directories(zapata-engine-automaton
  PUBLIC
    ${ENGINE_AUTOMATON_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-automaton
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-engine-fsm
)

set_target_properties(zapata-engine-automaton
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-engine-automaton-plugin SHARED)

target_sources(zapata-engine-automaton-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-engine-automaton-plugin
  PUBLIC
    ${ENGINE_AUTOMATON_INCLUDE_DIR}
)
target_link_libraries(zapata-engine-automaton-plugin
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-uri
    zapata-events
    zapata-engine-fsm
    zapata-engine-automaton
)

set_target_properties(zapata-engine-automaton-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-engine-automaton zapata-engine-automaton-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${ENGINE_AUTOMATON_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
