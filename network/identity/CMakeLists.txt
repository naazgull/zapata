add_library(zapata-net-identity-plugin SHARED)
add_dependencies(zapata-net-identity-plugin
  zapata-base
  zapata-engine-startup
  zapata-net-transport
  zapata-parser-json
  zapata-parser-uri
)
target_sources(zapata-net-identity-plugin
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-net-identity-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
)
target_link_libraries(zapata-net-identity-plugin
  PRIVATE
    zapata-base
    zapata-engine-startup
    zapata-net-transport
    zapata-parser-json
    zapata-parser-uri
 )
set_target_properties(zapata-net-identity-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
)

include(GNUInstallDirs)
install(TARGETS zapata-net-identity-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
