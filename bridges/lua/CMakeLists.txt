set(BRIDGE_LUA_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-bridge-lua SHARED)
add_dependencies(zapata-bridge-lua
  zapata-base
  zapata-parser-json
  zapata-bridge-base
)
target_sources(zapata-bridge-lua
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lua.cpp
  INTERFACE
    ${BRIDGE_LUA_INCLUDE_DIR}/zapata/lua.h
    ${BRIDGE_LUA_INCLUDE_DIR}/zapata/lua/lua.h
)
target_include_directories(zapata-bridge-lua
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    ${BRIDGE_LUA_INCLUDE_DIR}
  INTERFACE    
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-lua
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    ${LUA_LIBRARIES}
)
set_target_properties(zapata-bridge-lua
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-bridge-lua.so
)

add_library(zapata-bridge-lua-plugin SHARED)
add_dependencies(zapata-bridge-lua-plugin
  zapata-base
  zapata-parser-json
  zapata-events
  zapata-engine-startup
  zapata-bridge-base
  zapata-bridge-lua
)
target_sources(zapata-bridge-lua-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-bridge-lua-plugin
  PRIVATE
    zapata-base
    zapata-globals
    zapata-events
    zapata-lockfree
    zapata-parser-json
    zapata-engine-startup
    zapata-bridge-base
    zapata-bridge-lua
    ${BRIDGE_LUA_INCLUDE_DIR}
  INTERFACE
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-lua-plugin
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-events
    zapata-lockfree
    zapata-engine-startup
    zapata-bridge-base
    zapata-bridge-lua
    ${LUA_LIBRARIES}
)
set_target_properties(zapata-bridge-lua-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
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
add_dependencies(zapata_test_bridge_lua
  zapata-base
  zapata-parser-json
  zapata-bridge-base
  zapata-bridge-lua
)
target_sources(zapata_test_bridge_lua
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_bridge_lua
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-lua
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata_test_bridge_lua
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-bridge-base
    zapata-bridge-lua
    ${LUA_LIBRARIES}
)
set_target_properties(zapata_test_bridge_lua
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
