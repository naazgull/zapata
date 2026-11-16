set(COMMON_CONFIG_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-common-config SHARED)
add_dependencies(zapata-common-config
    zapata-base
    zapata-parser-json
)
target_sources(zapata-common-config
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
)
target_include_directories(zapata-common-config
 PRIVATE
    zapata-base
    zapata-parser-json
    ${COMMON_CONFIG_INCLUDE_DIR}
 INTERFACE
    $<BUILD_INTERFACE:${COMMON_CONFIG_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(zapata-common-config
  PUBLIC
    zapata-base
    zapata-parser-json
)
set_target_properties(zapata-common-config
  PROPERTIES
    EXPORT_NAME common-config
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-common-config.so
)

include(GNUInstallDirs)
install(TARGETS zapata-common-config
  EXPORT zapata
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_CONFIG_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
