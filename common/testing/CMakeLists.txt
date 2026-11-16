set(COMMON_TESTING_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Lua QUIET)

if(NOT Lua_FOUND)
  message(STATUS "zapata-common-testing skipped: Lua not found")
  return()
endif()

if(NOT TARGET Lua::Lua)
  add_library(Lua::Lua INTERFACE IMPORTED)
  set_target_properties(Lua::Lua
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
  )
endif()

include_directories(
  SYSTEM
    ${LUA_INCLUDE_DIR}
)

add_library(zapata-common-testing-plugin SHARED)
add_dependencies(zapata-common-testing-plugin
  zapata-base
  zapata-bridge-base
  zapata-bridge-lua
  zapata-engine-rest
  zapata-engine-startup
  zapata-events
  zapata-parser-json
  zapata-runtime
)
target_sources(zapata-common-testing-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-common-testing-plugin
  PRIVATE
    zapata-base
    zapata-bridge-base
    zapata-bridge-lua
    zapata-common-catalog
    zapata-engine-rest
    zapata-engine-startup
    zapata-engine-transport
    zapata-events
    zapata-globals
    zapata-lockfree
    zapata-parser-json
    zapata-runtime
    ${COMMON_TESTING_INCLUDE_DIR}
  INTERFACE
    $<BUILD_INTERFACE:${COMMON_TESTING_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(zapata-common-testing-plugin
  PUBLIC
    zapata-base
    zapata-bridge-base
    zapata-bridge-lua
    zapata-common-catalog
    zapata-engine-rest
    zapata-engine-startup
    zapata-engine-transport
    zapata-events
    zapata-globals
    zapata-lockfree
    zapata-parser-json
    zapata-runtime
    zapata-storage-sqlite
    Lua::Lua
)
set_target_properties(zapata-common-testing-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-common-testing-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_TESTING_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
