cmake_minimum_required(VERSION 3.16)

project(HTTPServer LANGUAGES CXX)

if((CMAKE_SIZEOF_VOID_P EQUAL 8) OR (CMAKE_CXX_COMPILER_ID MATCHES "SunPro"))
  set(IS64BIT 1)
  message("Generating 64bit code")
else()
  message("Generating 32bit code")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(Project_BUILD_DEBUG true)
else() #if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(Project_BUILD_DEBUG false)
endif()

#################################################### Config stuff ending ####################################################

set(CONAN_PROFILE "conan_profile")
if(UNIX)
    set(CONAN_PROFILE "${CONAN_PROFILE}_linux")
elseif(WIN32)
    set(CONAN_PROFILE "${CONAN_PROFILE}_windows")
endif()
if(IS64BIT)
	set(CONAN_PROFILE "${CONAN_PROFILE}_x64")
else()
	set(CONAN_PROFILE "${CONAN_PROFILE}_x86")
endif()
if(Project_BUILD_DEBUG)
	set(CONAN_PROFILE "${CONAN_PROFILE}_debug")
else()
	set(CONAN_PROFILE "${CONAN_PROFILE}_release")
endif()

set(CONAN_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/conan_toolchain.cmake")
if(EXISTS ${CONAN_TOOLCHAIN_FILE})
	include(${CONAN_TOOLCHAIN_FILE})  # Includes the contents of the conanbuildinfo.cmake file.
else()
	# Try building it
	set(CONAN_INSTALL_COMMAND "conan install -v --profile ./${CONAN_PROFILE} --build=missing .")
	execute_process(COMMAND ${CONAN_INSTALL_COMMAND}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	RESULT_VARIABLE CONAN_INSTALL_RESULT)

	if(CONAN_INSTALL_RESULT EQUAL 1)
		message(FATAL_ERROR "Error while handling Conan, stopping build!")
	endif()

	if(EXISTS ${CONAN_TOOLCHAIN_FILE}) # if succeeded
		include(${CONAN_TOOLCHAIN_FILE})  # Includes the contents of the conanbuildinfo.cmake file.
	else()
		message(ERROR " No conan_toolchain file found at '${CONAN_TOOLCHAIN_FILE}'. You might need to run:")
		message(FATAL_ERROR " ${CONAN_INSTALL_COMMAND}")
	endif()
endif()

set(CMAKE_GENERATOR_PLATFORM "" CACHE STRING "" FORCE) # Otherwise ninja fails
set(CMAKE_GENERATOR_TOOLSET "" CACHE STRING "" FORCE) # Otherwise ninja fails

##############################################################################################################################################################

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") # TODO: create the equivalent for other compilers?
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG:FULL /OPT:REF /OPT:ICF")
	endif()
endif()
message("Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(Boost_COMPILER gcc)
endif()

find_package(PostgreSQL REQUIRED)

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
	
include_directories(BEFORE
	${CMAKE_INCLUDE_PATH}
	"libs"
	"src") 

link_directories(BEFORE ${CMAKE_LIBRARY_PATH})
message("Link directories: ${CMAKE_LIBRARY_PATH}")

target_link_libraries(HTTPServer
	${PostgreSQL_LIBRARIES})

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
	string(REGEX REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REGEX REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

message("Compile flags: ${CMAKE_CXX_FLAGS}")

set_target_properties(HTTPServer PROPERTIES SUFFIX ${BINARY_SUFFIX})
message("Target extension is: ${BINARY_SUFFIX}")
