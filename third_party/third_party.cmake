add_library(mysqlcppconn8
  SHARED IMPORTED
)
if (NOT DONT_COMPILE_XDEVAPI)
  include(ExternalProject)
  include(ProcessorCount)
  processorcount(n)
  
  externalproject_add(
    mysqlxdevapi
    SOURCE_DIR "${THIRD_PARTY_DIR}/xdevapi"
    GIT_REPOSITORY "https://github.com/mysql/mysql-connector-cpp"
    GIT_TAG trunk
    CMAKE_ARGS
      -Wno-dev
      -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_BUILD_INSTALL_DIR}
      -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}
    BUILD_COMMAND
      ${CMAKE_COMMAND} --build <BINARY_DIR> -j${n}
  )
  add_dependencies(mysqlcppconn8
    mysqlxdevapi
  )
endif(NOT DONT_COMPILE_XDEVAPI)

set(MYSQL_XDEVAPI_INCLUDEDIR
  ${THIRD_PARTY_BUILD_INSTALL_DIR}/include
)
set(MYSQL_XDEVAPI_LIBDIR
  ${THIRD_PARTY_BUILD_INSTALL_DIR}/lib64/debug
)
set_target_properties(mysqlcppconn8
  PROPERTIES
    IMPORTED_LOCATION ${MYSQL_XDEVAPI_LIBDIR}/libmysqlcppconn8.so
)

include(GNUInstallDirs)
install(DIRECTORY ${THIRD_PARTY_BUILD_INSTALL_DIR}/include
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${THIRD_PARTY_BUILD_INSTALL_DIR}/lib
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.so*"
)
install(DIRECTORY ${THIRD_PARTY_BUILD_INSTALL_DIR}/lib
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.a*"
)
install(DIRECTORY ${THIRD_PARTY_BUILD_INSTALL_DIR}/lib64
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.so*"
)
install(DIRECTORY ${THIRD_PARTY_BUILD_INSTALL_DIR}/lib64
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.a*"
)
