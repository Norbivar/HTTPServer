cmake_minimum_required(VERSION 3.16)

project(HTTPServer LANGUAGES CXX)

find_package(openssl REQUIRED)
find_package(boost REQUIRED)
find_package(libpqxx REQUIRED)

add_definitions(-DFMT_HEADER_ONLY)

add_executable(HTTPServer 
"main.cpp"
"webserver.cpp"
"auth/auth.cpp"
"base/handle_request.cpp"
"base/http_session.cpp"
"base/listener.cpp"
"database/mappers/accounts_mapper.cpp"
"base/http_request.cpp"
"base/http_response.cpp"
"main.cpp"
"routing_table.cpp"
"session_tracker.cpp"
"database/sql/sql_handle.cpp"
"database/sql/sql_provider.cpp"
"database/sql/sql_manager.cpp"
"database/mappers/accounts_mapper.cpp"
"database/mappers/sessions_mapper.cpp" 
"database/session_element.cpp" 
"components/server/network_component.cpp"
"components/server/task_runner_component.cpp" 
"base/websocket_session.cpp")

include_directories(BEFORE
	SYSTEM
	${CMAKE_INCLUDE_PATH}
) 
	
include_directories(BEFORE
	"${MAIN_CMAKE_DIR}/libs"
	"."
) 

target_link_libraries(HTTPServer
	PRIVATE
	Libraries
	boost::boost
	openssl::openssl
	libpqxx::pqxx
)

SET_PROJECT_EXTENSION_AND_FLAGS(HTTPServer)