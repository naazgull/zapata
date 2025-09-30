ExternalProject_Add(zapata
  GIT_REPOSITORY    https://github.com/naazgull/zapata.git
  GIT_TAG           develop
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)

include_directories(
  SYSTEM
    ${CMAKE_INSTALL_PREFIX}/include
)

find_library(ZAPATA-BASE-LIB NAMES zapata-base
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-base SHARED IMPORTED)
add_dependencies(zapata-base
  zapata
)
set_target_properties(zapata-base
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-BASE-LIB}
)

find_library(ZAPATA-BRIDGE-BASE-LIB NAMES zapata-bridge-base
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-bridge-base SHARED IMPORTED)
add_dependencies(zapata-bridge-base
  zapata
)
set_target_properties(zapata-bridge-base
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-BRIDGE-BASE-LIB}
)

find_library(ZAPATA-BRIDGE-LUA-PLUGIN-LIB NAMES zapata-bridge-lua-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-bridge-lua-plugin SHARED IMPORTED)
add_dependencies(zapata-bridge-lua-plugin
  zapata
)
set_target_properties(zapata-bridge-lua-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-BRIDGE-LUA-PLUGIN-LIB}
)

find_library(ZAPATA-BRIDGE-LUA-LIB NAMES zapata-bridge-lua
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-bridge-lua SHARED IMPORTED)
add_dependencies(zapata-bridge-lua
  zapata
)
set_target_properties(zapata-bridge-lua
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-BRIDGE-LUA-LIB}
)

find_library(ZAPATA-BRIDGE-PROLOG-PLUGIN-LIB NAMES zapata-bridge-prolog-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-bridge-prolog-plugin SHARED IMPORTED)
add_dependencies(zapata-bridge-prolog-plugin
  zapata
)
set_target_properties(zapata-bridge-prolog-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-BRIDGE-PROLOG-PLUGIN-LIB}
)

find_library(ZAPATA-BRIDGE-PROLOG-LIB NAMES zapata-bridge-prolog
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-bridge-prolog SHARED IMPORTED)
add_dependencies(zapata-bridge-prolog
  zapata
)
set_target_properties(zapata-bridge-prolog
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-BRIDGE-PROLOG-LIB}
)

find_library(ZAPATA-COMMON-CATALOG-LIB NAMES zapata-common-catalog
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-common-catalog SHARED IMPORTED)
add_dependencies(zapata-common-catalog
  zapata
)
set_target_properties(zapata-common-catalog
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-COMMON-CATALOG-LIB}
)

find_library(ZAPATA-ENGINE-REST-PLUGIN-LIB NAMES zapata-engine-rest-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-engine-rest-plugin SHARED IMPORTED)
add_dependencies(zapata-engine-rest-plugin
  zapata
)
set_target_properties(zapata-engine-rest-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-ENGINE-REST-PLUGIN-LIB}
)

find_library(ZAPATA-ENGINE-REST-LIB NAMES zapata-engine-rest
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-engine-rest SHARED IMPORTED)
add_dependencies(zapata-engine-rest
  zapata
)
set_target_properties(zapata-engine-rest
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-ENGINE-REST-LIB}
)

find_library(ZAPATA-ENGINE-STARTUP-LIB NAMES zapata-engine-startup
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-engine-startup SHARED IMPORTED)
add_dependencies(zapata-engine-startup
  zapata
)
set_target_properties(zapata-engine-startup
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-ENGINE-STARTUP-LIB}
)

find_library(ZAPATA-ENGINE-TRANSPORT-PLUGIN-LIB NAMES zapata-engine-transport-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-engine-transport-plugin SHARED IMPORTED)
add_dependencies(zapata-engine-transport-plugin
  zapata
)
set_target_properties(zapata-engine-transport-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-ENGINE-TRANSPORT-PLUGIN-LIB}
)

find_library(ZAPATA-ENGINE-TRANSPORT-LIB NAMES zapata-engine-transport
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-engine-transport SHARED IMPORTED)
add_dependencies(zapata-engine-transport
  zapata
)
set_target_properties(zapata-engine-transport
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-ENGINE-TRANSPORT-LIB}
)

find_library(ZAPATA-EVENTS-LIB NAMES zapata-events
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-events SHARED IMPORTED)
add_dependencies(zapata-events
  zapata
)
set_target_properties(zapata-events
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-EVENTS-LIB}
)

find_library(ZAPATA-GENERATOR-AST-LIB NAMES zapata-generator-ast
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-generator-ast SHARED IMPORTED)
add_dependencies(zapata-generator-ast
  zapata
)
set_target_properties(zapata-generator-ast
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-GENERATOR-AST-LIB}
)

find_library(ZAPATA-GENERATOR-REST-LIB NAMES zapata-generator-rest
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-generator-rest SHARED IMPORTED)
add_dependencies(zapata-generator-rest
  zapata
)
set_target_properties(zapata-generator-rest
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-GENERATOR-REST-LIB}
)

find_library(ZAPATA-GLOBALS-LIB NAMES zapata-globals
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-globals SHARED IMPORTED)
add_dependencies(zapata-globals
  zapata
)
set_target_properties(zapata-globals
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-GLOBALS-LIB}
)

find_library(ZAPATA-IO-PIPE-LIB NAMES zapata-io-pipe
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-io-pipe SHARED IMPORTED)
add_dependencies(zapata-io-pipe
  zapata
)
set_target_properties(zapata-io-pipe
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-IO-PIPE-LIB}
)

find_library(ZAPATA-IO-SOCKET-LIB NAMES zapata-io-socket
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-io-socket SHARED IMPORTED)
add_dependencies(zapata-io-socket
  zapata
)
set_target_properties(zapata-io-socket
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-IO-SOCKET-LIB}
)

find_library(ZAPATA-IO-STREAM-LIB NAMES zapata-io-stream
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-io-stream SHARED IMPORTED)
add_dependencies(zapata-io-stream
  zapata
)
set_target_properties(zapata-io-stream
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-IO-STREAM-LIB}
)

find_library(ZAPATA-NET-HTTP-PLUGIN-LIB NAMES zapata-net-http-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-http-plugin SHARED IMPORTED)
add_dependencies(zapata-net-http-plugin
  zapata
)
set_target_properties(zapata-net-http-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-HTTP-PLUGIN-LIB}
)

find_library(ZAPATA-NET-HTTP-LIB NAMES zapata-net-http
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-http SHARED IMPORTED)
add_dependencies(zapata-net-http
  zapata
)
set_target_properties(zapata-net-http
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-HTTP-LIB}
)

find_library(ZAPATA-NET-IDENTITY-PLUGIN-LIB NAMES zapata-net-identity-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-identity-plugin SHARED IMPORTED)
add_dependencies(zapata-net-identity-plugin
  zapata
)
set_target_properties(zapata-net-identity-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-IDENTITY-PLUGIN-LIB}
)

find_library(ZAPATA-NET-LOCAL-PLUGIN-LIB NAMES zapata-net-local-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-local-plugin SHARED IMPORTED)
add_dependencies(zapata-net-local-plugin
  zapata
)
set_target_properties(zapata-net-local-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-LOCAL-PLUGIN-LIB}
)

find_library(ZAPATA-NET-LOCAL-LIB NAMES zapata-net-local
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-local SHARED IMPORTED)
add_dependencies(zapata-net-local
  zapata
)
set_target_properties(zapata-net-local
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-LOCAL-LIB}
)

find_library(ZAPATA-NET-PIPE-PLUGIN-LIB NAMES zapata-net-pipe-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-pipe-plugin SHARED IMPORTED)
add_dependencies(zapata-net-pipe-plugin
  zapata
)
set_target_properties(zapata-net-pipe-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-PIPE-PLUGIN-LIB}
)

find_library(ZAPATA-NET-PIPE-LIB NAMES zapata-net-pipe
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-pipe SHARED IMPORTED)
add_dependencies(zapata-net-pipe
  zapata
)
set_target_properties(zapata-net-pipe
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-PIPE-LIB}
)

find_library(ZAPATA-NET-SELF-PLUGIN-LIB NAMES zapata-net-self-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-self-plugin SHARED IMPORTED)
add_dependencies(zapata-net-self-plugin
  zapata
)
set_target_properties(zapata-net-self-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-SELF-PLUGIN-LIB}
)

find_library(ZAPATA-NET-SELF-LIB NAMES zapata-net-self
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-self SHARED IMPORTED)
add_dependencies(zapata-net-self
  zapata
)
set_target_properties(zapata-net-self
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-SELF-LIB}
)

find_library(ZAPATA-NET-TCP-PLUGIN-LIB NAMES zapata-net-tcp-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-tcp-plugin SHARED IMPORTED)
add_dependencies(zapata-net-tcp-plugin
  zapata
)
set_target_properties(zapata-net-tcp-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-TCP-PLUGIN-LIB}
)

find_library(ZAPATA-NET-TCP-LIB NAMES zapata-net-tcp
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-tcp SHARED IMPORTED)
add_dependencies(zapata-net-tcp
  zapata
)
set_target_properties(zapata-net-tcp
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-TCP-LIB}
)

find_library(ZAPATA-NET-TRANSPORT-LIB NAMES zapata-net-transport
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-transport SHARED IMPORTED)
add_dependencies(zapata-net-transport
  zapata
)
set_target_properties(zapata-net-transport
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-TRANSPORT-LIB}
)

find_library(ZAPATA-NET-UPNP-PLUGIN-LIB NAMES zapata-net-upnp-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-upnp-plugin SHARED IMPORTED)
add_dependencies(zapata-net-upnp-plugin
  zapata
)
set_target_properties(zapata-net-upnp-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-UPNP-PLUGIN-LIB}
)

find_library(ZAPATA-NET-UPNP-LIB NAMES zapata-net-upnp
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-upnp SHARED IMPORTED)
add_dependencies(zapata-net-upnp
  zapata
)
set_target_properties(zapata-net-upnp
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-UPNP-LIB}
)

find_library(ZAPATA-NET-WEBSOCKET-PLUGIN-LIB NAMES zapata-net-websocket-plugin
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-websocket-plugin SHARED IMPORTED)
add_dependencies(zapata-net-websocket-plugin
  zapata
)
set_target_properties(zapata-net-websocket-plugin
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-WEBSOCKET-PLUGIN-LIB}
)

find_library(ZAPATA-NET-WEBSOCKET-LIB NAMES zapata-net-websocket
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-net-websocket SHARED IMPORTED)
add_dependencies(zapata-net-websocket
  zapata
)
set_target_properties(zapata-net-websocket
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-NET-WEBSOCKET-LIB}
)

find_library(ZAPATA-ONTOLOGY-LIB NAMES zapata-ontology
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-ontology SHARED IMPORTED)
add_dependencies(zapata-ontology
  zapata
)
set_target_properties(zapata-ontology
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-ONTOLOGY-LIB}
)

find_library(ZAPATA-PARSER-FUNCTIONAL-LIB NAMES zapata-parser-functional
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-parser-functional SHARED IMPORTED)
add_dependencies(zapata-parser-functional
  zapata
)
set_target_properties(zapata-parser-functional
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-PARSER-FUNCTIONAL-LIB}
)

find_library(ZAPATA-PARSER-HTTP-LIB NAMES zapata-parser-http
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-parser-http SHARED IMPORTED)
add_dependencies(zapata-parser-http
  zapata
)
set_target_properties(zapata-parser-http
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-PARSER-HTTP-LIB}
)

find_library(ZAPATA-PARSER-JSON-LIB NAMES zapata-parser-json
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-parser-json SHARED IMPORTED)
add_dependencies(zapata-parser-json
  zapata
)
set_target_properties(zapata-parser-json
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-PARSER-JSON-LIB}
)

find_library(ZAPATA-PARSER-URI-LIB NAMES zapata-parser-uri
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-parser-uri SHARED IMPORTED)
add_dependencies(zapata-parser-uri
  zapata
)
set_target_properties(zapata-parser-uri
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-PARSER-URI-LIB}
)

find_library(ZAPATA-REGEX-GRAPH-LIB NAMES zapata-regex-graph
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-regex-graph SHARED IMPORTED)
add_dependencies(zapata-regex-graph
  zapata
)
set_target_properties(zapata-regex-graph
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-REGEX-GRAPH-LIB}
)

find_library(ZAPATA-REST-CLIENT-EXAMPLE-LIB NAMES zapata-rest-client-example
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-rest-client-example SHARED IMPORTED)
add_dependencies(zapata-rest-client-example
  zapata
)
set_target_properties(zapata-rest-client-example
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-REST-CLIENT-EXAMPLE-LIB}
)

find_library(ZAPATA-REST-EXAMPLE-LIB NAMES zapata-rest-example
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-rest-example SHARED IMPORTED)
add_dependencies(zapata-rest-example
  zapata
)
set_target_properties(zapata-rest-example
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-REST-EXAMPLE-LIB}
)

find_library(ZAPATA-STORAGE-CONNECTOR-LIB NAMES zapata-storage-connector
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-storage-connector SHARED IMPORTED)
add_dependencies(zapata-storage-connector
  zapata
)
set_target_properties(zapata-storage-connector
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-STORAGE-CONNECTOR-LIB}
)

find_library(ZAPATA-STORAGE-MYSQLX-LIB NAMES zapata-storage-mysqlx
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-storage-mysqlx SHARED IMPORTED)
add_dependencies(zapata-storage-mysqlx
  zapata
)
set_target_properties(zapata-storage-mysqlx
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-STORAGE-MYSQLX-LIB}
)

find_library(ZAPATA-STORAGE-SQLITE-LIB NAMES zapata-storage-sqlite
  PATHS
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_SYSTEM_ENVIRONMENT_PATH
)
add_library(zapata-storage-sqlite SHARED IMPORTED)
add_dependencies(zapata-storage-sqlite
  zapata
)
set_target_properties(zapata-storage-sqlite
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA-STORAGE-SQLITE-LIB}
)
