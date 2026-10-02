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
  INTERFACE
    ${COMMON_CONFIG_INCLUDE_DIR}/zapata/config/config.h
    ${COMMON_CONFIG_INCLUDE_DIR}/zapata/config.h
)
target_include_directories(zapata-common-config
 PRIVATE
    zapata-base
    zapata-parser-json
    ${COMMON_CONFIG_INCLUDE_DIR}
 INTERFACE
    ${COMMON_CONFIG_INCLUDE_DIR}
)
target_link_libraries(zapata-common-config
  PRIVATE
    zapata-base
    zapata-parser-json
)
set_target_properties(zapata-common-config
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-common-config.so
)

include(GNUInstallDirs)
install(TARGETS zapata-common-config
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_CONFIG_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
