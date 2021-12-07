#include "sql_handle.hpp"

#include <jdbc/cppconn/connection.h>
#include <jdbc/mysql_connection.h>
#include "sql_manager.hpp"

sql_handle::sql_handle(sql_manager& man, std::shared_ptr<sql::Connection> from) :
	m_connection{from},
	m_connection_as_raw_api{static_cast<sql::mysql::MySQL_Connection*>(from.get())},
	manager{man}
{ }

sql_handle::~sql_handle()
{

	if (!m_connection->getAutoCommit())
		m_connection->commit();

	manager.add_handle(m_connection);
}

sql::SQLString sql_handle::escape(const std::string& str)
{
	return m_connection_as_raw_api->escapeString(str);
}