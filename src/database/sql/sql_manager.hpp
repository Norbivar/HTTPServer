#pragma once

#include <string>
#include <memory>
#include <vector>

#include <rigtorp/MPMCQueue.h>
#include "sql_handle.hpp"

class sql_manager
{
public:
	sql_manager();
	~sql_manager() = default;

	sql_handle acquire_handle(); // threadsafe
	void add_handle(std::shared_ptr<pqxx::connection> conn); // threadsafe

private:
	const std::string m_sql_address;
	const std::uint16_t m_sql_port;
	const std::string m_sql_database;
	const std::string m_sql_user;
	const std::string m_sql_pass;
	const std::uint8_t m_sql_connection_pool_size = 3;

	rigtorp::MPMCQueue<std::shared_ptr<pqxx::connection>> m_connection_pool;
};