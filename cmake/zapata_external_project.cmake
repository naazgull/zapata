include(ProcessorCount)
ProcessorCount(nproc)

ExternalProject_Add(zapata
  GIT_REPOSITORY    https://github.com/naazgull/zapata.git
  GIT_TAG           develop
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DWITH_ASAN=${WITH_ASAN}
    -DWITH_UBSAN=${WITH_UBSAN}
    -DWITH_EXCEPTION_PROPAGATION=${WITH_EXCEPTION_PROPAGATION}
    -DWITH_ALLOCATOR_DEBUGGING=${WITH_ALLOCATOR_DEBUGGING}
)

include_directories(
  SYSTEM
    ${CMAKE_INSTALL_PREFIX}/include
)

add_library(zapata-base SHARED IMPORTED)
add_dependencies(zapata-base
  zapata
)
set_target_properties(zapata-base
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-base.so
)

add_library(zapata-bridge-base SHARED IMPORTED)
add_dependencies(zapata-bridge-base
  zapata
)
set_target_properties(zapata-bridge-base
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-bridge-base.so
)

add_library(zapata-bridge-lua SHARED IMPORTED)
add_dependencies(zapata-bridge-lua
  zapata
)
set_target_properties(zapata-bridge-lua
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-bridge-lua.so
)

add_library(zapata-bridge-prolog SHARED IMPORTED)
add_dependencies(zapata-bridge-prolog
  zapata
)
set_target_properties(zapata-bridge-prolog
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-bridge-prolog.so
)

add_library(zapata-common-catalog SHARED IMPORTED)
add_dependencies(zapata-common-catalog
  zapata
)
set_target_properties(zapata-common-catalog
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-common-catalog.so
)

add_library(zapata-engine-rest SHARED IMPORTED)
add_dependencies(zapata-engine-rest
  zapata
)
set_target_properties(zapata-engine-rest
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-engine-rest.so
)

add_library(zapata-engine-startup SHARED IMPORTED)
add_dependencies(zapata-engine-startup
  zapata
)
set_target_properties(zapata-engine-startup
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-engine-startup.so
)

add_library(zapata-engine-transport SHARED IMPORTED)
add_dependencies(zapata-engine-transport
  zapata
)
set_target_properties(zapata-engine-transport
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-engine-transport.so
)

add_library(zapata-events SHARED IMPORTED)
add_dependencies(zapata-events
  zapata
)
set_target_properties(zapata-events
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-events.so
)

add_library(zapata-globals SHARED IMPORTED)
add_dependencies(zapata-globals
  zapata
)
set_target_properties(zapata-globals
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-globals.so
)

add_library(zapata-io-pipe SHARED IMPORTED)
add_dependencies(zapata-io-pipe
  zapata
)
set_target_properties(zapata-io-pipe
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-io-pipe.so
)

add_library(zapata-io-socket SHARED IMPORTED)
add_dependencies(zapata-io-socket
  zapata
)
set_target_properties(zapata-io-socket
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-io-socket.so
)

add_library(zapata-io-stream SHARED IMPORTED)
add_dependencies(zapata-io-stream
  zapata
)
set_target_properties(zapata-io-stream
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-io-stream.so
)

add_library(zapata-net-http SHARED IMPORTED)
add_dependencies(zapata-net-http
  zapata
)
set_target_properties(zapata-net-http
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-http.so
)

add_library(zapata-net-local SHARED IMPORTED)
add_dependencies(zapata-net-local
  zapata
)
set_target_properties(zapata-net-local
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-local.so
)

add_library(zapata-net-pipe SHARED IMPORTED)
add_dependencies(zapata-net-pipe
  zapata
)
set_target_properties(zapata-net-pipe
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-pipe.so
)

add_library(zapata-net-self SHARED IMPORTED)
add_dependencies(zapata-net-self
  zapata
)
set_target_properties(zapata-net-self
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-self.so
)

add_library(zapata-net-tcp SHARED IMPORTED)
add_dependencies(zapata-net-tcp
  zapata
)
set_target_properties(zapata-net-tcp
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-tcp.so
)

add_library(zapata-net-transport SHARED IMPORTED)
add_dependencies(zapata-net-transport
  zapata
)
set_target_properties(zapata-net-transport
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-transport.so
)

add_library(zapata-net-upnp SHARED IMPORTED)
add_dependencies(zapata-net-upnp
  zapata
)
set_target_properties(zapata-net-upnp
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-upnp.so
)

add_library(zapata-net-websocket SHARED IMPORTED)
add_dependencies(zapata-net-websocket
  zapata
)
set_target_properties(zapata-net-websocket
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-websocket.so
)

add_library(zapata-ontology SHARED IMPORTED)
add_dependencies(zapata-ontology
  zapata
)
set_target_properties(zapata-ontology
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-ontology.so
)

add_library(zapata-parser-functional SHARED IMPORTED)
add_dependencies(zapata-parser-functional
  zapata
)
set_target_properties(zapata-parser-functional
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-parser-functional.so
)

add_library(zapata-parser-http SHARED IMPORTED)
add_dependencies(zapata-parser-http
  zapata
)
set_target_properties(zapata-parser-http
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-parser-http.so
)

add_library(zapata-parser-json SHARED IMPORTED)
add_dependencies(zapata-parser-json
  zapata
)
set_target_properties(zapata-parser-json
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-parser-json.so
)

add_library(zapata-parser-uri SHARED IMPORTED)
add_dependencies(zapata-parser-uri
  zapata
)
set_target_properties(zapata-parser-uri
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-parser-uri.so
)

add_library(zapata-regex-graph SHARED IMPORTED)
add_dependencies(zapata-regex-graph
  zapata
)
set_target_properties(zapata-regex-graph
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-regex-graph.so
)

add_library(zapata-runtime SHARED IMPORTED)
add_dependencies(zapata-runtime
  zapata
)
set_target_properties(zapata-runtime
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-runtime.so
)

add_library(zapata-storage-connector SHARED IMPORTED)
add_dependencies(zapata-storage-connector
  zapata
)
set_target_properties(zapata-storage-connector
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-storage-connector.so
)

add_library(zapata-storage-mongodb SHARED IMPORTED)
add_dependencies(zapata-storage-mongodb
  zapata
)
set_target_properties(zapata-storage-mongodb
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-storage-mongodb.so
)

add_library(zapata-storage-mysqlx SHARED IMPORTED)
add_dependencies(zapata-storage-mysqlx
  zapata
)
set_target_properties(zapata-storage-mysqlx
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-storage-mysqlx.so
)

add_library(zapata-storage-pgsql SHARED IMPORTED)
add_dependencies(zapata-storage-pgsql
  zapata
)
set_target_properties(zapata-storage-pgsql
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-storage-pgsql.so
)

add_library(zapata-storage-sqlite SHARED IMPORTED)
add_dependencies(zapata-storage-sqlite
  zapata
)
set_target_properties(zapata-storage-sqlite
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-storage-sqlite.so
)
