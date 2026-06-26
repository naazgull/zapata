set(STORAGE_MONGODB_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Mongocxx 3.2 QUIET)
find_package(Mongoc QUIET)

if(NOT (Mongocxx_FOUND AND Mongoc_FOUND))
  message(STATUS "zapata-storage-mongodb skipped: MongoDB C++ driver (mongocxx/mongoc) not found")
  return()
endif()

add_library(zapata-storage-mongodb SHARED)
add_dependencies(zapata-storage-mongodb
  zapata-base
  zapata-parser-json
  zapata-net-http
)
target_sources(zapata-storage-mongodb
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mongodb/Client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mongodb/convert_mongo.cpp
  INTERFACE
    ${STORAGE_MONGODB_INCLUDE_DIR}/zapata/mongodb.h
    ${STORAGE_MONGODB_INCLUDE_DIR}/zapata/mongodb/Client.h
    ${STORAGE_MONGODB_INCLUDE_DIR}/zapata/mongodb/convert_mongo.h
)
target_include_directories(zapata-storage-mongodb
  PRIVATE
    zapata-base
    zapata-parser-json
    zapata-net-http
    ${STORAGE_MONGODB_INCLUDE_DIR}
  INTERFACE
    ${STORAGE_MONGODB_INCLUDE_DIR}
)
target_link_libraries(zapata-storage-mongodb
  PUBLIC
    ${MONGOCXX_LIBRARIES}
    ${MONGOC_LIBRARIES}
    zapata-base
    zapata-parser-json
    zapata-net-http
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
