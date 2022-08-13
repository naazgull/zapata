set(BRIDGE_LUA_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-bridge-lua SHARED)

target_sources(zapata-bridge-lua
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lua.cpp
    ${BRIDGE_LUA_INCLUDE_DIR}/zapata/lua.h
    ${BRIDGE_LUA_INCLUDE_DIR}/zapata/lua/lua.h
)
target_include_directories(zapata-bridge-lua
  PUBLIC
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-lua
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    ${LUA_LIBRARIES}
)

set_target_properties(zapata-bridge-lua
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-bridge-lua-plugin SHARED)

target_sources(zapata-bridge-lua-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-bridge-lua-plugin
  PUBLIC
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-lua-plugin
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-lua
    ${LUA_LIBRARIES}
)

set_target_properties(zapata-bridge-lua-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-bridge-lua zapata-bridge-lua-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${BRIDGE_LUA_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_bridge_lua)

target_sources(zapata_test_bridge_lua
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_bridge_lua
  PRIVATE
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata_test_bridge_lua
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-lua
    ${LUA_LIBRARIESPython3_LIBRARIES}
)

set_target_properties(zapata_test_bridge_lua
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
