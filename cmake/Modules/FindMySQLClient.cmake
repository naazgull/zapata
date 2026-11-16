# FindMySQLClient.cmake
#
# Locates the MySQL C client library and its headers, and provides the
# imported target MySQLClient::MySQLClient.
#
# Result variables:
#   MySQLClient_FOUND        - True if both the library and headers were found.
#   MYSQLCLIENT_LIBRARIES    - Full path to libmysqlclient.
#   MYSQLCLIENT_INCLUDE_DIRS - Directory containing <mysql/mysql.h>.

find_path(MYSQLCLIENT_INCLUDE_DIR NAMES mysql/mysql.h)
find_library(MYSQLCLIENT_LIBRARIES NAMES mysqlclient)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySQLClient
  REQUIRED_VARS MYSQLCLIENT_LIBRARIES MYSQLCLIENT_INCLUDE_DIR
)

if(MySQLClient_FOUND AND NOT TARGET MySQLClient::MySQLClient)
  set(MYSQLCLIENT_INCLUDE_DIRS "${MYSQLCLIENT_INCLUDE_DIR}")
  add_library(MySQLClient::MySQLClient UNKNOWN IMPORTED)
  set_target_properties(MySQLClient::MySQLClient PROPERTIES
    IMPORTED_LOCATION "${MYSQLCLIENT_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${MYSQLCLIENT_INCLUDE_DIR}"
  )
endif()
