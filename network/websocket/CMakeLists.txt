set(NETWORK_WEBSOCKET_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-net-websocket SHARED)
add_dependencies(zapata-net-websocket
  zapata-base
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-websocket
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/websocket.cpp
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata/net/websocket.h
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata/net/transport/websocket.h
)
target_include_directories(zapata-net-websocket
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-socket,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata-net-websocket
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-globals,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-socket,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
)
set_target_properties(zapata-net-websocket
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-net-websocket.so
)

add_library(zapata-net-websocket-plugin SHARED)
add_dependencies(zapata-net-websocket-plugin
  zapata-base
  zapata-engine-startup
  zapata-events
  zapata-globals
  zapata-io-stream
  zapata-io-socket
  zapata-lockfree
  zapata-net-transport
  zapata-net-tcp
  zapata-parser-json
  zapata-parser-uri
  zapata-net-websocket
)
target_sources(zapata-net-websocket-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-websocket-plugin
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-engine-startup,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-events,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-globals,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-stream,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-io-socket,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-lockfree,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-net-transport,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-uri,INTERFACE_INCLUDE_DIRECTORIES>
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}
  INTERFACE
    ${NETWORK_WEBSOCKET_INCLUDE_DIR}
)
target_link_libraries(zapata-net-websocket-plugin
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-engine-startup,LIBRARIES>
    $<TARGET_PROPERTY:zapata-events,LIBRARIES>
    $<TARGET_PROPERTY:zapata-globals,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-stream,LIBRARIES>
    $<TARGET_PROPERTY:zapata-io-socket,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-transport,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-uri,LIBRARIES>
    $<TARGET_PROPERTY:zapata-net-websocket,LIBRARIES>
)
set_target_properties(zapata-net-websocket-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-net-websocket zapata-net-websocket-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${NETWORK_WEBSOCKET_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
