set(ENGINE_EVENTS_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
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
    zapata-engine-startup
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
    zapata-engine-startup
    ${ENGINE_EVENTS_INCLUDE_DIR}
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
    zapata-engine-startup
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
