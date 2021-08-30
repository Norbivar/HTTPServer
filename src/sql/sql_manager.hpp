#pragma once

#include <string>
#include <memory>
#include <vector>

#include <rigtorp/MPMCQueue.h>
#include "sql_handle.hpp"

namespace sql{ class Driver; class Connection; }

class sql_manager
{
public:
	sql_manager();
	~sql_manager() = default;

	sql_handle acquire_handle(); // threadsafe
	void add_handle(std::shared_ptr<sql::Connection> conn); // threadsafe

private:
	const std::string m_mysql_address;
	const std::string m_mysql_database;
	const std::string m_mysql_user;
	const std::string m_mysql_pass;

	const std::uint8_t m_connection_pool_size = 3;

	rigtorp::MPMCQueue<std::shared_ptr<sql::Connection>> m_connection_pool;
	sql::Driver* m_driver;
};