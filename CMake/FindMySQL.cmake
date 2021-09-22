
set(WITH_CONCPP $ENV{WITH_CONCPP} CACHE PATH
  "MySQL Connector/C++ 8.0 install location"
)

set(CONCPP_INCLUDE_DIR $ENV{CONCPP_INCLUDE_DIR} CACHE PATH
  "Location of Connector/C++ 8.0 headers"
)

set(CONCPP_LIB_DIR $ENV{CONCPP_LIB_DIR} CACHE PATH
  "Location of Connector/C++ 8.0 libraries"
)

# Set include and library paths if not given

if(WITH_CONCPP)
  if(NOT CONCPP_INCLUDE_DIR)
     set(CONCPP_INCLUDE_DIR "${WITH_CONCPP}/include")
  endif()
  if(NOT CONCPP_LIB_DIR)
    if(IS64BIT AND EXISTS "${WITH_CONCPP}/lib64")
      set(CONCPP_LIB_DIR "${WITH_CONCPP}/lib64")
    else()
      set(CONCPP_LIB_DIR "${WITH_CONCPP}/lib")
    endif()
  endif()
endif()

# Location for static libraries (differs from base location on Windows)

set(CONCPP_STATIC_LIB_DIR "${CONCPP_LIB_DIR}")
if(WIN32)
  set(VS "vs14")
  set(CONCPP_STATIC_LIB_DIR "${CONCPP_LIB_DIR}/vs14")
endif()


if(NOT CONCPP_INCLUDE_DIR OR NOT CONCPP_LIB_DIR)
  message(FATAL_ERROR
    "This project requires MySQL Connector/C++ 8.0, please specify install location"
    " using WITH_CONCPP setting or set header/library paths with CONCPP_INCLUDE_DIR"
    " and CONCPP_LIB_DIR settings."
  )
endif()

if(NOT EXISTS "${CONCPP_INCLUDE_DIR}/mysqlx/xdevapi.h")
  message(FATAL_ERROR
    "Could not find MySQL Connector/C++ 8.0 headers at specified"
    " location: ${CONCPP_INCLUDE_DIR}"
  )
endif()


set(WITH_SSL $ENV{WITH_SSL} CACHE STRING
  "Set to 'builtin' if connector was built with built-in SSL support"
)

if(NOT WITH_SSL MATCHES "^(system|yes)$")

  if(EXISTS ${WITH_SSL}/include/openssl/ssl.h)
    set(OPENSSL_ROOT_DIR  "${WITH_SSL}")
  endif()

endif()

find_package(OpenSSL)

if(OPENSSL_FOUND)
  MESSAGE(STATUS "OPENSSL_VERSION = ${OPENSSL_VERSION}")
  MESSAGE(STATUS "OPENSSL_SSL_LIBRARY = ${OPENSSL_SSL_LIBRARY}")
  MESSAGE(STATUS "OPENSSL_CRYPTO_LIBRARY = ${OPENSSL_CRYPTO_LIBRARY}")
endif()

option(WITH_JDBC "Also build the JDBC API test application" OFF)

option(SQLCONN_BUILD_STATIC "Link statically with the connector library" OFF)

if(SQLCONN_BUILD_STATIC)
  message("Linking statically")
endif()

option(STATIC_MSVCRT "Use static MSVC runtime library" OFF)

if(STATIC_MSVCRT)
  message("Using static runtime library.")
else()
  message("Using dynamic runtime library.")
endif()


# ========================================================================
# Dependencies

#
# Find Connector/C++ libraries
#
# Installation layout is as follows
#
# On Windows the install layout is as follows, where NN is the MSVC version
# used to build the connector, A is the major ABI version:
#
#  {lib,lib64}/mysqlcppconn-A-vsNN.dll            <-- shared library
#  {lib,lib64}/vsNN/mysqlcppconn-static.lib       <-- static with /MD
#  {lib,lib64}/vsNN/mysqlcppconn-static-mt.lib    <-- static with /MT
#  {lib,lib64}/vsNN/mysqlcppconn.lib              <-- import library for DLL
#
# On Linux it is as follows, where X.Y.Z is the connector version
#
#  {lib,lib64}/libmysqlcppconn.so.A.X.Y.Z         <-- shared library
#  {lib,lib64}/libmysqlcppconn.so.A               <-- soname link
#  {lib,lib64}/libmysqlcppconn.so                 <-- development link
#  {lib,lib64}/libmysqlcppconn-static.a          <-- static library
#
# Additionally, if connector is built in debug mode, the libraries are installed
# in debug/ subfolder of {lib,lib64}/ or {lib,lib64}/vsNN/.
#

set(find_name mysqlcppconn)
set(find_dir  "${CONCPP_LIB_DIR}")

#
# Note: On Windows we link with the import library located in the static
# library dir and named the same as the shared library.
#

if(SQLCONN_BUILD_STATIC OR WIN32)
  set(find_dir  ${CONCPP_STATIC_LIB_DIR})
endif()

if(SQLCONN_BUILD_STATIC)
  set(find_name mysqlcppconn8-static)
  if(WIN32 AND STATIC_MSVCRT)
    set(find_name "${find_name}-mt")
  endif()
endif()

#message("-- looking for: ${find_name}")
#message("-- looking in: ${find_dir}")

# This will cause find_libary() to perform search each time.

set(CONCPP_LIB force-NOTFOUND CACHE PATH "" FORCE)
set(CONCPP_LIB_DEBUG force-NOTFOUND CACHE BOOL "" FORCE)

find_library(CONCPP_LIB
  NAMES ${find_name}
  PATHS "${find_dir}"
  NO_DEFAULT_PATH
)

find_library(CONCPP_LIB_DEBUG
  NAMES ${find_name}
  PATHS "${find_dir}/debug"
  NO_DEFAULT_PATH
)


if(NOT CONCPP_LIB AND NOT CONCPP_LIB_DEBUG)
  message(FATAL_ERROR
    "Could not find Connector/C++ libraries at: ${find_dir}"
  )
endif()

message("Using connector lib at: ${CONCPP_LIB}")