include(ExternalProject)
include(ProcessorCount)
processorcount(n)

find_library(MYSQL_XDEVAPI_LIB NAMES mysqlcppconn8
  PATHS
    ${THIRD_PARTY_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /usr/local/lib64
)

add_library(mysqlcppconn8
  SHARED IMPORTED
)

if (NOT MYSQL_XDEVAPI_LIB)
  message("-- XDevAPI client API library not found, downloading and compiling")
  
  set(MYSQL_XDEVAPI_LIB ${THIRD_PARTY_INSTALL_PREFIX}/lib/libmysqlcppconn8.so)
  
  externalproject_add(
    mysqlxdevapi
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/third_party/xdevapi"
    GIT_REPOSITORY "https://github.com/mysql/mysql-connector-cpp"
    GIT_TAG trunk
    BUILD_BYPRODUCTS
      ${MYSQL_XDEVAPI_LIB}
    CMAKE_ARGS
      -Wno-dev
      -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_PREFIX}
      -DCMAKE_INSTALL_LIBDIR=${THIRD_PARTY_INSTALL_PREFIX}/lib
      -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}
      -DCMAKE_BUILD_TYPE=Release
      -DWITH_DEBUG=0
    BUILD_COMMAND
      ${CMAKE_COMMAND} --build <BINARY_DIR> -j${n}
  )
  add_dependencies(mysqlcppconn8
    mysqlxdevapi
  )
else()
  message("-- Found XDevAPI: ${MYSQL_XDEVAPI_LIB}")
endif()

set_target_properties(mysqlcppconn8
  PROPERTIES
    IMPORTED_LOCATION ${MYSQL_XDEVAPI_LIB}
)

include(GNUInstallDirs)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/include
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/lib
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.so*"
)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/lib
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.a*"
)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/lib64
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.so*"
)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/lib64
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.a*"
)
