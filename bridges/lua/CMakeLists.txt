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
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_LUA_INCLUDE_DIR}
  INTERFACE    
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-lua
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
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
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-events,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-engine-startup,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-lua,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_LUA_INCLUDE_DIR}
  INTERFACE
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata-bridge-lua-plugin
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
    $<TARGET_PROPERTY:zapata-lockfree,LIBRARIES>
    $<TARGET_PROPERTY:zapata-engine-startup,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-lua,LIBRARIES>
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
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-bridge-lua,INTERFACE_INCLUDE_DIRECTORIES>
    ${BRIDGE_LUA_INCLUDE_DIR}
)
target_link_libraries(zapata_test_bridge_lua
  PUBLIC
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-bridge-lua,LIBRARIES>
    ${LUA_LIBRARIES}
)
set_target_properties(zapata_test_bridge_lua
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
