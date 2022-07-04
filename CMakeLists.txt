cmake_minimum_required(VERSION 3.18)

project(zapata VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ZAPATA_COMPILE_FLAGS "-O3 -DNDEBUG -fno-omit-frame-pointer")
set(ZAPATA_LINK_FLAGS "")

string(TOLOWER ${CMAKE_BUILD_TYPE} ZAPATA_BUILD_TYPE)
if(ZAPATA_BUILD_TYPE STREQUAL debug)
  set(ZAPATA_COMPILE_FLAGS "-O0 -ggdb3 -fno-omit-frame-pointer")
  set(ZAPATA_LINK_FLAGS "")
endif()

message("-- Compiler flags: ${ZAPATA_COMPILE_FLAGS}")
message("-- Linker flags: ${ZAPATA_LINK_FLAGS}")

find_package(Python3 COMPONENTS Interpreter Development)
find_package(Lua)
find_package(OpenSSL)
find_package(SQLite3)
find_library(LMDB_LIBS NAMES lmdb
  PATHS
    /usr/lib
    /usr/lib64
    /usr/lib//x86_64-linux-gnu/
    /usr/local/lib
    /usr/local/lib64
)

include(third_party/third_party.cmake)

include_directories(
  ${Python3_INCLUDE_DIRS}
  ${LUA_INCLUDE_DIR}
  ${SQLite3_INCLUDE_DIRS}
  ${MYSQL_XDEVAPI_INCLUDEDIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/common/globals/include
  ${CMAKE_CURRENT_SOURCE_DIR}/common/base/include
  ${CMAKE_CURRENT_SOURCE_DIR}/common/events/include
  ${CMAKE_CURRENT_SOURCE_DIR}/common/graph/include
  ${CMAKE_CURRENT_SOURCE_DIR}/common/allocator/include
  ${CMAKE_CURRENT_SOURCE_DIR}/common/lockfree/include
  ${CMAKE_CURRENT_SOURCE_DIR}/common/catalogue/include
  ${CMAKE_CURRENT_SOURCE_DIR}/bridges/base/include
  ${CMAKE_CURRENT_SOURCE_DIR}/bridges/lisp/include
  ${CMAKE_CURRENT_SOURCE_DIR}/bridges/python/include
  ${CMAKE_CURRENT_SOURCE_DIR}/bridges/lua/include
  ${CMAKE_CURRENT_SOURCE_DIR}/bridges/prolog/include
  ${CMAKE_CURRENT_SOURCE_DIR}/engines/startup/include
  ${CMAKE_CURRENT_SOURCE_DIR}/engines/automaton/include
  ${CMAKE_CURRENT_SOURCE_DIR}/engines/rest/include
  ${CMAKE_CURRENT_SOURCE_DIR}/engines/pipeline/include
  ${CMAKE_CURRENT_SOURCE_DIR}/engines/drops/include
  ${CMAKE_CURRENT_SOURCE_DIR}/engines/fsm/include
  ${CMAKE_CURRENT_SOURCE_DIR}/engines/dom/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/upnp/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/local/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/websocket/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/tcp/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/mqtt/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/zmq/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/pipe/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/http/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/smtp/include
  ${CMAKE_CURRENT_SOURCE_DIR}/network/transport/include
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/postgresql/include
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/lmdb/include
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/mysqlx/include
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/connector/include
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/couchdb/include
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/sqlite/include
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/redis/include
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/mongodb/include
  ${CMAKE_CURRENT_SOURCE_DIR}/io/socket/include
  ${CMAKE_CURRENT_SOURCE_DIR}/io/pipe/include
  ${CMAKE_CURRENT_SOURCE_DIR}/io/stream/include
  ${CMAKE_CURRENT_SOURCE_DIR}/parsers/json/include
  ${CMAKE_CURRENT_SOURCE_DIR}/parsers/uri/include
  ${CMAKE_CURRENT_SOURCE_DIR}/parsers/http/include
  ${CMAKE_CURRENT_SOURCE_DIR}/parsers/functional/include
  ${CMAKE_CURRENT_SOURCE_DIR}/security/oauth2/include
)

add_subdirectory(common/base)
add_subdirectory(common/globals)
add_subdirectory(common/allocator)
add_subdirectory(common/graph)
add_subdirectory(common/lockfree)
add_subdirectory(common/events)
add_subdirectory(io/stream)
add_subdirectory(io/pipe)
add_subdirectory(io/socket)
add_subdirectory(parsers/json)
add_subdirectory(parsers/functional)
add_subdirectory(parsers/uri)
add_subdirectory(parsers/http)
add_subdirectory(network/transport)
# add_subdirectory(engines/startup)
# add_subdirectory(bridges/base)
# add_subdirectory(bridges/lua)
# add_subdirectory(bridges/prolog)
# add_subdirectory(bridges/python)
# add_subdirectory(network/http)
# add_subdirectory(network/local)
# add_subdirectory(network/pipe)
# add_subdirectory(network/tcp)
# add_subdirectory(network/websocket)
# add_subdirectory(storage/connector)
# add_subdirectory(storage/mysqlx)
# add_subdirectory(storage/lmdb)
# add_subdirectory(storage/sqlite)
# add_subdirectory(common/catalogue)
# add_subdirectory(engines/pipeline)
# add_subdirectory(engines/rest)
# add_subdirectory(engines/fsm)
# add_subdirectory(engines/automaton)
# add_subdirectory(engines/dom)
# add_subdirectory(engines/tasks)
# add_subdirectory(security/oauth2)

# add_subdirectory(network/mqtt)
# add_subdirectory(network/smtp)
# add_subdirectory(network/upnp)
# add_subdirectory(storage/couchdb)
# add_subdirectory(storage/mongodb)
# add_subdirectory(storage/postgresql)
# add_subdirectory(storage/redis)

# ------------------------------------------------------------------------
# TARGET: uninstall
#
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
