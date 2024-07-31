cmake_minimum_required(VERSION 3.16)

include("CMakeFuncs.txt")

project(Main)

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
else()
	message(ERROR "Unsupported target architecture (cannot decide on Conan files to use)!")
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

set(CONAN_TOOLCHAIN_OUTPUT_BUILD_FOLDER "build/${CONAN_PROFILE}")
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

SET(MAIN_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory("libs")
add_subdirectory("src/server")
add_subdirectory("src/load_balancer")