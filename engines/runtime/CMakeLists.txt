set(ENGINE_RUNTIME_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-runtime SHARED)
add_dependencies(zapata-runtime
    zapata-base
    zapata-events
    zapata-engine-startup
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
)
target_sources(zapata-runtime
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime.cpp
  INTERFACE
    ${ENGINE_RUNTIME_INCLUDE_DIR}/zapata/runtime.h
)
target_include_directories(zapata-runtime
  PRIVATE
    zapata-base
    zapata-events
    zapata-engine-startup
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    ${ENGINE_RUNTIME_INCLUDE_DIR}
  INTERFACE
    ${ENGINE_RUNTIME_INCLUDE_DIR}
)
target_link_libraries(zapata-runtime
  PUBLIC
    zapata-base
    zapata-events
    zapata-engine-startup
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
)
set_target_properties(zapata-runtime
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-runtime.so
)

include(GNUInstallDirs)
install(TARGETS zapata-runtime
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_EXECDIR}
)
install(DIRECTORY ${ENGINE_RUNTIME_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zpt)
add_dependencies(zpt
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-runtime
)
target_sources(zpt
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/zpt.cpp
)
target_include_directories(zpt
  PRIVATE
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-lockfree
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-runtime
    ${ENGINE_RUNTIME_INCLUDE_DIR}
)
target_link_libraries(zpt
  PRIVATE
    zapata-base
    zapata-events
    zapata-globals
    zapata-io-stream
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
    zapata-runtime
)
set_target_properties(zpt
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zpt
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_EXECDIR}
)
