cmake_minimum_required(VERSION 3.16)

project(LoadBalancer LANGUAGES CXX)

find_package(openssl REQUIRED)
find_package(boost REQUIRED)

add_definitions(-DFMT_HEADER_ONLY)

add_executable(LoadBalancer 
	"main.cpp"
)

include_directories(BEFORE
	SYSTEM
	${CMAKE_INCLUDE_PATH}
) 
	
include_directories(BEFORE
	"${MAIN_CMAKE_DIR}/libs"
	"."
) 

target_link_libraries(LoadBalancer
	PRIVATE
	Libraries
	boost::boost
	openssl::openssl
)

SET_PROJECT_EXTENSION_AND_FLAGS(LoadBalancer)