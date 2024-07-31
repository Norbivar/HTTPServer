cmake_minimum_required(VERSION 3.16)

project(Libraries LANGUAGES CXX)

find_package(boost QUIET REQUIRED)

message("Setting up libraries project")

add_definitions(-DFMT_HEADER_ONLY)

add_library(Libraries 
"Config.cpp"
"Logger.cpp")

target_link_libraries(Libraries
	PRIVATE
	boost::boost
)

include_directories(
	SYSTEM
	${CMAKE_INCLUDE_PATH}
) 
	
include_directories(AFTER
	"."
)

set_property(TARGET Libraries PROPERTY CXX_STANDARD 20)
message("Libraries project set up!")