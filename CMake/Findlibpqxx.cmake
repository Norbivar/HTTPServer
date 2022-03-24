
set(libpqxx_DIR $ENV{LIBPQXX_ROOT} CACHE PATH
  "Libpqxx (C++ PostgreS Connector) install location"
)
message("PostgreS C++ connector searching with path: ${libpqxx_DIR}")

set(LIBPQXX_INCLUDE_DIR $ENV{CONCPP_INCLUDE_DIR} CACHE PATH
  "Location of Libpqxx (C++ PostgreS Connector) headers"
)

set(LIBPQXX_LIB_DIR $ENV{CONCPP_LIB_DIR} CACHE PATH
  "Location of Libpqxx (C++ PostgreS Connector) libraries"
)

# Set include and library paths if not given
if(NOT LIBPQXX_INCLUDE_DIR)
    set(LIBPQXX_INCLUDE_DIR "${libpqxx_DIR}/include")
endif()
if(NOT LIBPQXX_LIB_DIR)
    set(LIBPQXX_LIB_DIR "${libpqxx_DIR}/lib")
endif()

message("PostgreS C++ connector include dir: ${LIBPQXX_INCLUDE_DIR}")
message("PostgreS C++ connector lib dir: ${LIBPQXX_LIB_DIR}")


 file(GLOB LIBPQXX_LIBRARIES
    "${LIBPQXX_LIB_DIR}/*.lib"
    "${LIBPQXX_LIB_DIR}/*.a"
)

# Location for static libraries (differs from base location on Windows)
message("PostgreS C++ connector libs: ${LIBPQXX_LIBRARIES}")