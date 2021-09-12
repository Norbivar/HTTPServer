#pragma once

#include <memory>
#include <vector>
#include <condition_variable>
#include <shared_mutex>

#include <jdbc/cppconn/sqlstring.h>

namespace sql
{ 
	namespace mysql { class MySQL_Connection; }
	class Connection; 
}
class sql_manager;

class sql_handle
{
public:
	sql::Connection* operator->() { return m_connection.get(); }
	const sql::Connection* operator->() const { return m_connection.get(); }
	sql::SQLString escape(const std::string& str);

	sql_handle(sql_manager& man, std::shared_ptr<sql::Connection> from);
	~sql_handle();

private:
	std::shared_ptr<sql::Connection> m_connection;
	sql::mysql::MySQL_Connection* m_connection_as_raw_api;
	sql_manager& manager;
};