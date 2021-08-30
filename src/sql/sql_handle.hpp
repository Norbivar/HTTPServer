#pragma once

#include <memory>
#include <vector>
#include <condition_variable>
#include <shared_mutex>

namespace sql{ class Connection; }
class sql_manager;

class sql_handle
{
public:
	sql::Connection* operator->() { return m_connection.get(); }

	sql_handle(sql_manager& man, std::shared_ptr<sql::Connection> from);
	~sql_handle();

private:
	std::shared_ptr<sql::Connection> m_connection;
	sql_manager& manager;
};