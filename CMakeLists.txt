project(HTTPServer LANGUAGES CXX)
cmake_minimum_required(VERSION 3.16)

##############################################################################################################################################################

if((CMAKE_SIZEOF_VOID_P EQUAL 8) OR (CMAKE_CXX_COMPILER_ID MATCHES "SunPro"))
  set(IS64BIT 1)
  message("Generating 64bit code")
else()
  message("Generating 32bit code")
endif()

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") # TODO: create the equivalent for other compilers?
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG:FULL /OPT:REF /OPT:ICF")
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(Project_BUILD_DEBUG true)
else() #if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(Project_BUILD_DEBUG false)
endif()

####### Build configuration INDEPENDENT things:
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(Boost_COMPILER gcc)
endif()
find_package(Boost REQUIRED COMPONENTS filesystem date_time random regex) 
find_package(OpenSSL REQUIRED) 

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/")
find_package(PostgreSQL REQUIRED)
find_package(libpqxx REQUIRED)

###########################################################################
add_definitions(-DFMT_HEADER_ONLY)

add_executable(HTTPServer 
"src/main.cpp"
"libs/Config.cpp"
"libs/Logger.cpp"
"src/webserver.cpp"
"src/auth/auth.cpp"
"src/base/handle_request.cpp"
"src/base/http_session.cpp"
"src/base/listener.cpp"
"src/database/mappers/accounts_mapper.cpp"
"src/base/http_request.cpp"
"src/base/http_response.cpp"
"src/main.cpp"
"src/routing_table.cpp"
"src/session_tracker.cpp"
"src/database/sql/sql_handle.cpp"
"src/database/sql/sql_provider.cpp"
"src/database/sql/sql_manager.cpp"
"src/database/mappers/accounts_mapper.cpp"
"src/database/mappers/sessions_mapper.cpp" 
"src/database/session_element.cpp")
	
include_directories(BEFORE ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${LIBPQXX_INCLUDE_DIR} "libs" "src") 
target_link_libraries(HTTPServer ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${LIBPQXX_LIBRARIES} ${PostgreSQL_LIBRARIES})

# Set the binary extension
set(BINARY_SUFFIX "")

set_property(TARGET HTTPServer PROPERTY CXX_STANDARD 20)
if(UNIX)
    target_link_libraries(HTTPServer pthread)
    set(BINARY_SUFFIX ".out")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
elseif(WIN32)
    set(BINARY_SUFFIX ".exe")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /D_WIN32_WINNT=0x0601 /wd4996 /wd4702 /utf-8")
	string(REGEX REPLACE "/W[3|4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()
set_target_properties(HTTPServer PROPERTIES SUFFIX ${BINARY_SUFFIX})
message("Target extension is: ${BINARY_SUFFIX}")
