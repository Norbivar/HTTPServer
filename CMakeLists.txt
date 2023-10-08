cmake_minimum_required(VERSION 3.16)

project(HTTPServer LANGUAGES CXX)

function(RERUN_CONAN_INSTALL VAR_CONAN_PROFILE VAR_BUILD_TYPE VAR_CONAN_OUTPUT_DIR)
	# Try building it
	set(CONAN_INSTALL_COMMAND "conan")
	set(CONAN_INSTALL_PARAMS 
		"install" 
		"-s" "build_type=${VAR_BUILD_TYPE}"
		"-pr:h" "./${VAR_CONAN_PROFILE}"
		"--output-folder=${VAR_CONAN_OUTPUT_DIR}"
		"--build=missing"
		".")

	string(REPLACE ";" " " CONAN_INSTALL_PARAMS_STRING "${CONAN_INSTALL_PARAMS}")
	message(STATUS "Running Conan command: ${CONAN_INSTALL_COMMAND} ${CONAN_INSTALL_PARAMS_STRING}")
	execute_process(COMMAND ${CONAN_INSTALL_COMMAND} ${CONAN_INSTALL_PARAMS}
					WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
					RESULT_VARIABLE CONAN_INSTALL_RESULT
					OUTPUT_STRIP_TRAILING_WHITESPACE)

	#message("rv='${CONAN_INSTALL_RESULT}'")
	if(${CONAN_INSTALL_RESULT} EQUAL 1)
		message(FATAL_ERROR "Error while handling Conan, stopping build!")
	endif()
endfunction()

#######################################################################################################

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_PROFILE "")
set(CONAN_BUILD_TYPE_STR "")
if((CMAKE_SIZEOF_VOID_P EQUAL 8) OR (CMAKE_CXX_COMPILER_ID MATCHES "SunPro"))
  set(IS64BIT 1)
endif()

if(UNIX)
    set(BUILD_PROFILE "linux")
elseif(WIN32)
    set(BUILD_PROFILE "windows")
endif()
if(IS64BIT)
	set(BUILD_PROFILE "${BUILD_PROFILE}_x64")
else()
	set(BUILD_PROFILE "${BUILD_PROFILE}_x86")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(BUILD_PROFILE "${BUILD_PROFILE}_debug")
	set(CONAN_BUILD_TYPE_STR "Debug")
else()
	set(BUILD_PROFILE "${BUILD_PROFILE}_release")
	set(CONAN_BUILD_TYPE_STR "Release")
endif()
set(CONAN_PROFILE "conan_profile_${BUILD_PROFILE}")

message(STATUS "Build type: ${CONAN_BUILD_TYPE_STR}")
message(STATUS "Using Conan profile: ${CONAN_PROFILE}")

SET(CONAN_TOOLCHAIN_OUTPUT_BUILD_FOLDER "build/${CONAN_PROFILE}")
set(CONAN_TOOLCHAIN_EXPECTED_PATH "${CMAKE_SOURCE_DIR}/${CONAN_TOOLCHAIN_OUTPUT_BUILD_FOLDER}/")
set(CONAN_TOOLCHAIN_FILE "${CONAN_TOOLCHAIN_EXPECTED_PATH}/conan_toolchain.cmake")

if(${CMAKE_TOOLCHAIN_FILE}) # include the toolchain if given
	include(${CMAKE_TOOLCHAIN_FILE})
endif()

if(EXISTS ${CONAN_TOOLCHAIN_FILE})
	include(${CONAN_TOOLCHAIN_FILE})
	set(CMAKE_TOOLCHAIN_FILE "${CONAN_TOOLCHAIN_FILE}")
else()
	RERUN_CONAN_INSTALL(${CONAN_PROFILE} ${CONAN_BUILD_TYPE_STR} ${CONAN_TOOLCHAIN_EXPECTED_PATH})

	if(EXISTS ${CONAN_TOOLCHAIN_FILE}) # if succeeded
		include(${CONAN_TOOLCHAIN_FILE})
		set(CMAKE_TOOLCHAIN_FILE "${CONAN_TOOLCHAIN_FILE}")
	else()
		message(ERROR " Failed to automatically produce conan_toolchain to '${CONAN_TOOLCHAIN_FILE}'. You might need to run manually:")
		message(FATAL_ERROR " ${CONAN_INSTALL_COMMAND} ")
	endif()
endif()

unset(CMAKE_GENERATOR_PLATFORM CACHE)
unset(CMAKE_GENERATOR_TOOLSET CACHE)

message("Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") # TODO: create the equivalent for other compilers?
	set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /DEBUG:FULL /OPT:REF /OPT:ICF")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(Boost_COMPILER gcc)
endif()

###########################################################################
find_package(openssl REQUIRED)
find_package(boost REQUIRED)
find_package(libpqxx REQUIRED)

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
"src/database/session_element.cpp" 
"src/components/server/network_component.cpp"
"src/components/server/task_runner_component.cpp" "src/base/websocket_session.cpp")
	
include_directories(BEFORE
	${CMAKE_INCLUDE_PATH}
	"libs"
	"src") 

target_link_libraries(HTTPServer
	PRIVATE
	boost::boost
	openssl::openssl
	libpqxx::pqxx
)

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
	string(REGEX REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

message("Compile flags: ${CMAKE_CXX_FLAGS}")

set_target_properties(HTTPServer PROPERTIES SUFFIX ${BINARY_SUFFIX})
message("Target extension is: ${BINARY_SUFFIX}")