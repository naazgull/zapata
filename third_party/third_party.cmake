include(ExternalProject)
include(ProcessorCount)
processorcount(n)

externalproject_add(
  mysqlxdevapi
  GIT_REPOSITORY "https://github.com/mysql/mysql-connector-cpp"
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/xdevapi/download
  BUILD_COMMAND make -j${N}
  CMAKE_ARGS
    -Wno-dev
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
set(MYSQL_XDEVAPI_INCLUDEDIR
  ${CMAKE_INSTALL_PREFIX}/include
)
set(MYSQL_XDEVAPI_LIBDIR
  ${CMAKE_INSTALL_PREFIX}/lib64/debug
)
add_library(mysqlcppconn8
  SHARED IMPORTED
)
add_dependencies(mysqlcppconn8
  mysqlxdevapi
)
set_target_properties(mysqlcppconn8
  PROPERTIES
    IMPORTED_LOCATION ${MYSQL_XDEVAPI_LIBDIR}/libmysqlcppconn8.so
)

  
