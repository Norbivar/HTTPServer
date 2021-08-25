project(HTTPServer LANGUAGES CXX)
cmake_minimum_required(VERSION 3.16)

##############################################################################################################################################################

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") # TODO: create the equivalent for other compilers?
		SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG:FULL /OPT:REF /OPT:ICF")
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(Project_BUILD_DEBUG true)
else() #if(CMAKE_BUILD_TYPE STREQUAL "Release")
	SET(Project_BUILD_DEBUG false)
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
find_package(MySQL REQUIRED)

file(GLOB_RECURSE All_SRC
    "src/*.cpp"
)

file(GLOB_RECURSE Libs_SRC
    "libs/*.cpp"
)

###########################################################################
add_executable(HTTPServer ${All_SRC} ${Libs_SRC})

include_directories(BEFORE ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} "libs") 
target_link_libraries(HTTPServer ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${MYSQL_LIBRARIES})

# Set the binary extension
set(BINARY_SUFFIX "")

set_property(TARGET HTTPServer PROPERTY CXX_STANDARD 17)
if(UNIX)
    target_link_libraries(HTTPServer pthread)
    set(BINARY_SUFFIX ".out")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
elseif(WIN32)
    set(BINARY_SUFFIX ".exe")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /D_WIN32_WINNT=0x0601 /wd4996 /wd4702")
	string(REGEX REPLACE "/W[3|4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()
set_target_properties(HTTPServer PROPERTIES SUFFIX ${BINARY_SUFFIX})
message("Target extension is: ${BINARY_SUFFIX}")
