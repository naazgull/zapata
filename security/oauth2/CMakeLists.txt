set(SECURITY_OAUTH2_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-security-oauth2 SHARED)

target_sources(zapata-security-oauth2
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/oauth2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/listeners.cpp
  PUBLIC
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/oauth2.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2/listeners.h
    ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata/oauth2.h
)
target_include_directories(zapata-security-oauth2
  PUBLIC
    ${SECURITY_OAUTH2_INCLUDE_DIR}
)
target_link_libraries(zapata-security-oauth2
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-engine-rest
)

set_target_properties(zapata-security-oauth2
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-security-oauth2-plugin SHARED)

target_sources(zapata-security-oauth2-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-security-oauth2-plugin
  PUBLIC
    ${SECURITY_OAUTH2_INCLUDE_DIR}
)
target_link_libraries(zapata-security-oauth2-plugin
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-engine-rest
    zapata-security-oauth2
)

set_target_properties(zapata-security-oauth2-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-security-oauth2 zapata-security-oauth2-plugin
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${SECURITY_OAUTH2_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)
