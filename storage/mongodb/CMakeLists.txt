set(STORAGE_MONGODB_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(mongocxx QUIET)
find_package(bsoncxx QUIET)

if(NOT mongocxx_FOUND OR NOT bsoncxx_FOUND)
  message(STATUS "zapata-storage-mongodb skipped: mongocxx/bsoncxx not found")
  return()
endif()

add_library(zapata-storage-mongodb SHARED)
add_dependencies(zapata-storage-mongodb
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
)
target_sources(zapata-storage-mongodb
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/translate.cpp
  INTERFACE
    ${STORAGE_MONGODB_INCLUDE_DIR}/zapata/mongodb.h
    ${STORAGE_MONGODB_INCLUDE_DIR}/zapata/mongodb/translate.h
    ${STORAGE_MONGODB_INCLUDE_DIR}/zapata/mongodb/connector.h
)
target_include_directories(zapata-storage-mongodb
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_MONGODB_INCLUDE_DIR}
  INTERFACE
    ${STORAGE_MONGODB_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-mongodb
  PUBLIC
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
)
set_target_properties(zapata-storage-mongodb
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzapata-storage-mongodb.so
)

include(GNUInstallDirs)
install(TARGETS zapata-storage-mongodb
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${STORAGE_MONGODB_INCLUDE_DIR}/zapata
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

add_executable(zapata_example_storage_mongodb)
add_dependencies(zapata_example_storage_mongodb
  zapata-base
  zapata-parser-json
  zapata-parser-functional
  zapata-storage-connector
  zapata-storage-mongodb
)
target_sources(zapata_example_storage_mongodb
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/mongodb.cpp
)
target_include_directories(zapata_example_storage_mongodb
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    ${STORAGE_MONGODB_INCLUDE_DIR}
)
target_link_libraries(zapata_example_storage_mongodb
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-parser-functional
    zapata-storage-connector
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
    zapata-storage-mongodb
)
set_target_properties(zapata_example_storage_mongodb
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)
