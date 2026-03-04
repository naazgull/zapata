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

add_library(zapata-bridge-lua-plugin SHARED IMPORTED)
add_dependencies(zapata-bridge-lua-plugin
  zapata
)
set_target_properties(zapata-bridge-lua-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-bridge-lua-plugin.so
)

add_library(zapata-bridge-lua SHARED IMPORTED)
add_dependencies(zapata-bridge-lua
  zapata
)
set_target_properties(zapata-bridge-lua
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-bridge-lua.so
)

add_library(zapata-bridge-prolog-plugin SHARED IMPORTED)
add_dependencies(zapata-bridge-prolog-plugin
  zapata
)
set_target_properties(zapata-bridge-prolog-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-bridge-prolog-plugin.so
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

add_library(zapata-runtime SHARED IMPORTED)
add_dependencies(zapata-runtime
  zapata
)
set_target_properties(zapata-runtime
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-runtime.so
)

add_library(zapata-engine-rest-plugin SHARED IMPORTED)
add_dependencies(zapata-engine-rest-plugin
  zapata
)
set_target_properties(zapata-engine-rest-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-engine-rest-plugin.so
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

add_library(zapata-engine-transport-plugin SHARED IMPORTED)
add_dependencies(zapata-engine-transport-plugin
  zapata
)
set_target_properties(zapata-engine-transport-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-engine-transport-plugin.so
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

add_library(zapata-generator-ast SHARED IMPORTED)
add_dependencies(zapata-generator-ast
  zapata
)
set_target_properties(zapata-generator-ast
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-generator-ast.so
)

add_library(zapata-generator-rest SHARED IMPORTED)
add_dependencies(zapata-generator-rest
  zapata
)
set_target_properties(zapata-generator-rest
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-generator-rest.so
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

add_library(zapata-net-http-plugin SHARED IMPORTED)
add_dependencies(zapata-net-http-plugin
  zapata
)
set_target_properties(zapata-net-http-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-http-plugin.so
)

add_library(zapata-net-http SHARED IMPORTED)
add_dependencies(zapata-net-http
  zapata
)
set_target_properties(zapata-net-http
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-http.so
)

add_library(zapata-net-identity-plugin SHARED IMPORTED)
add_dependencies(zapata-net-identity-plugin
  zapata
)
set_target_properties(zapata-net-identity-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-identity-plugin.so
)

add_library(zapata-net-local-plugin SHARED IMPORTED)
add_dependencies(zapata-net-local-plugin
  zapata
)
set_target_properties(zapata-net-local-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-local-plugin.so
)

add_library(zapata-net-local SHARED IMPORTED)
add_dependencies(zapata-net-local
  zapata
)
set_target_properties(zapata-net-local
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-local.so
)

add_library(zapata-net-pipe-plugin SHARED IMPORTED)
add_dependencies(zapata-net-pipe-plugin
  zapata
)
set_target_properties(zapata-net-pipe-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-pipe-plugin.so
)

add_library(zapata-net-pipe SHARED IMPORTED)
add_dependencies(zapata-net-pipe
  zapata
)
set_target_properties(zapata-net-pipe
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-pipe.so
)

add_library(zapata-net-self-plugin SHARED IMPORTED)
add_dependencies(zapata-net-self-plugin
  zapata
)
set_target_properties(zapata-net-self-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-self-plugin.so
)

add_library(zapata-net-self SHARED IMPORTED)
add_dependencies(zapata-net-self
  zapata
)
set_target_properties(zapata-net-self
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-self.so
)

add_library(zapata-net-tcp-plugin SHARED IMPORTED)
add_dependencies(zapata-net-tcp-plugin
  zapata
)
set_target_properties(zapata-net-tcp-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-tcp-plugin.so
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

add_library(zapata-net-upnp-plugin SHARED IMPORTED)
add_dependencies(zapata-net-upnp-plugin
  zapata
)
set_target_properties(zapata-net-upnp-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-upnp-plugin.so
)

add_library(zapata-net-upnp SHARED IMPORTED)
add_dependencies(zapata-net-upnp
  zapata
)
set_target_properties(zapata-net-upnp
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-upnp.so
)

add_library(zapata-net-websocket-plugin SHARED IMPORTED)
add_dependencies(zapata-net-websocket-plugin
  zapata
)
set_target_properties(zapata-net-websocket-plugin
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-net-websocket-plugin.so
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

add_library(zapata-rest-client-example SHARED IMPORTED)
add_dependencies(zapata-rest-client-example
  zapata
)
set_target_properties(zapata-rest-client-example
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-rest-client-example.so
)

add_library(zapata-rest-example SHARED IMPORTED)
add_dependencies(zapata-rest-example
  zapata
)
set_target_properties(zapata-rest-example
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-rest-example.so
)

add_library(zapata-storage-connector SHARED IMPORTED)
add_dependencies(zapata-storage-connector
  zapata
)
set_target_properties(zapata-storage-connector
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-storage-connector.so
)

add_library(zapata-storage-mysqlx SHARED IMPORTED)
add_dependencies(zapata-storage-mysqlx
  zapata
)
set_target_properties(zapata-storage-mysqlx
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-storage-mysqlx.so
)

add_library(zapata-storage-sqlite SHARED IMPORTED)
add_dependencies(zapata-storage-sqlite
  zapata
)
set_target_properties(zapata-storage-sqlite
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libzapata-storage-sqlite.so
)
