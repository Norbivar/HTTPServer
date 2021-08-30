#include "sql_handle.hpp"

#include <jdbc/cppconn/connection.h>
#include "sql_manager.hpp"

sql_handle::sql_handle(sql_manager& man, std::shared_ptr<sql::Connection> from) :
	m_connection{from},
	manager{man}
{ }

sql_handle::~sql_handle()
{
	manager.add_handle(m_connection);
}