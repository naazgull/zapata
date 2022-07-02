set(MYSQL_XDEVAPI_SRCDIR
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/xdevapi
)
set(MYSQL_XDEVAPI_INCLUDEDIR
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/xdevapi/include
)
set(MYSQL_XDEVAPI_LIBDIR
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/xdevapi/lib64/debug
)
add_library(mysqlcppconn8
  SHARED IMPORTED
)
set_target_properties(mysqlcppconn8
  PROPERTIES
    IMPORTED_LOCATION ${MYSQL_XDEVAPI_LIBDIR}/libmysqlcppconn8.so
)
