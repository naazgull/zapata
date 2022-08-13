set(COMMON_CATALOGUE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-common-catalogue SHARED)

target_sources(zapata-common-catalogue
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/catalogue.cpp
    ${COMMON_CATALOGUE_INCLUDE_DIR}/zapata/catalogue/catalogue.h
    ${COMMON_CATALOGUE_INCLUDE_DIR}/zapata/catalogue.h
)
target_include_directories(zapata-common-catalogue
  PUBLIC
    ${COMMON_CATALOGUE_INCLUDE_DIR}
)
target_link_libraries(zapata-common-catalogue
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-storage-sqlite
)

set_target_properties(zapata-common-catalogue
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

add_library(zapata-common-catalogue-plugin SHARED)

target_sources(zapata-common-catalogue-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-common-catalogue-plugin
  PUBLIC
    ${COMMON_CATALOGUE_INCLUDE_DIR}
)
target_link_libraries(zapata-common-catalogue-plugin
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-storage-sqlite
    zapata-common-catalogue
)

set_target_properties(zapata-common-catalogue-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS zapata-common-catalogue
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${COMMON_CATALOGUE_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_test_common_catalogue)

target_sources(zapata_test_common_catalogue
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)
target_include_directories(zapata_test_common_catalogue
  PRIVATE
    ${COMMON_CATALOGUE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_catalogue
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-storage-sqlite
    zapata-common-catalogue
)

set_target_properties(zapata_test_common_catalogue
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
