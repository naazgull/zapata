set(COMMON_CATALOGUE_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(zapata-common-catalogue SHARED)
add_dependencies(zapata-common-catalogue
    zapata-base
    zapata-parser-json
    zapata-storage-connector
    zapata-storage-sqlite
)
target_sources(zapata-common-catalogue
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/catalogue.cpp
    ${COMMON_CATALOGUE_INCLUDE_DIR}/zapata/catalogue/catalogue.h
    ${COMMON_CATALOGUE_INCLUDE_DIR}/zapata/catalogue.h
)
target_include_directories(zapata-common-catalogue
 PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-connector,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-sqlite,INTERFACE_INCLUDE_DIRECTORIES>
    ${COMMON_CATALOGUE_INCLUDE_DIR}
 INTERFACE
    ${COMMON_CATALOGUE_INCLUDE_DIR}
)
target_link_libraries(zapata-common-catalogue
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-storage-connector,LIBRARIES>
    ${SQLite3_LIBRARIES}
    $<TARGET_PROPERTY:zapata-storage-sqlite,LIBRARIES>
)
set_target_properties(zapata-common-catalogue
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-common-catalogue.so
)

add_library(zapata-common-catalogue-plugin SHARED)
add_dependencies(zapata-common-catalogue-plugin
    zapata-base
    zapata-parser-json
    zapata-storage-sqlite
    zapata-common-catalogue
)
target_sources(zapata-common-catalogue-plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
target_include_directories(zapata-common-catalogue-plugin
 PRIVATE
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-sqlite,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-connector,INTERFACE_INCLUDE_DIRECTORIES>
    ${COMMON_CATALOGUE_INCLUDE_DIR}
 INTERFACE
    ${COMMON_CATALOGUE_INCLUDE_DIR}
)
target_link_libraries(zapata-common-catalogue-plugin
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-storage-connector,LIBRARIES>
    ${SQLite3_LIBRARIES}
    $<TARGET_PROPERTY:zapata-storage-sqlite,LIBRARIES>
    $<TARGET_PROPERTY:zapata-common-catalogue,LIBRARIES>
)
set_target_properties(zapata-common-catalogue-plugin
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-common-catalogue-plugin.so
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
    $<TARGET_PROPERTY:zapata-base,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-parser-json,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-sqlite,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:zapata-storage-connector,INTERFACE_INCLUDE_DIRECTORIES>
    ${COMMON_CATALOGUE_INCLUDE_DIR}
)
target_link_libraries(zapata_test_common_catalogue
  PRIVATE
    $<TARGET_PROPERTY:zapata-base,LIBRARIES>
    $<TARGET_PROPERTY:zapata-parser-json,LIBRARIES>
    $<TARGET_PROPERTY:zapata-storage-connector,LIBRARIES>
    ${SQLite3_LIBRARIES}
    $<TARGET_PROPERTY:zapata-storage-sqlite,LIBRARIES>
    $<TARGET_PROPERTY:zapata-common-catalogue,LIBRARIES>
)
set_target_properties(zapata_test_common_catalogue
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
