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

set(ZPT_MODULES
  zapata-base
  zapata-bridge-base
  zapata-bridge-lua
  zapata_bridge_prolog_bindings
  zapata-bridge-prolog
  zapata-common-catalog
  zapata-engine-rest
  zapata-engine-startup
  zapata-engine-transport
  zapata-events
  zapata-example-rest-consumer
  zapata-example-rest-provider
  zapata-example-ws-provider
  zapata-generator-ast
  zapata-generator-rest
  zapata-globals
  zapata-io-pipe
  zapata-io-socket
  zapata-io-stream
  zapata-net-http
  zapata-net-local
  zapata-net-pipe
  zapata-net-self
  zapata-net-tcp
  zapata-net-transport
  zapata-net-upnp
  zapata-net-websocket
  zapata-ontology
  zapata-parser-functional
  zapata-parser-http
  zapata-parser-json
  zapata-parser-uri
  zapata-regex-graph
  zapata-runtime
  zapata-storage-connector
  zapata-storage-mongodb
  zapata-storage-mysqlx
  zapata-storage-pgsql
  zapata-storage-sqlite
)

foreach(ZPT_MODULE IN LISTS ZPT_MODULES)
  add_library(${ZPT_MODULE} SHARED IMPORTED)
  add_dependencies(${ZPT_MODULE}
    zapata
  )
  set_target_properties(${ZPT_MODULE}
    PROPERTIES
      IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/lib${ZPT_MODULE}.so
  )  
endforeach()
