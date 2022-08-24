#pragma once

#include <memory>
#include <vector>

#include <pqxx/connection>
#include <pqxx/version>

#include <Logging>
#include <Config>

#include "sql_handle.hpp"
#include "sql_provider.hpp"

class sql_manager
{
public:
	sql_manager(const std::string& name, const sql_provider& provider);

	~sql_manager() = default;

	sql_handle acquire_handle(); // threadsafe

	void add_handle(std::shared_ptr<pqxx::connection> conn); // threadsafe

private:
	void push_new_handle();

	const std::string m_name;

	const std::string m_sql_address;
	const std::uint16_t m_sql_port;
	const std::string m_sql_database;
	const std::string m_sql_user;
	const std::string m_sql_pass;
	const std::uint8_t m_sql_connection_desired_pool_size;
	const std::uint8_t m_sql_connection_max_pool_size;
	const std::chrono::milliseconds m_sql_connection_pool_expand_time;

	std::mutex m_connection_pool_mutex;
	std::condition_variable m_connection_pool_push_event;

	std::vector<std::shared_ptr<pqxx::connection>> m_connection_pool;
	std::uint8_t m_sql_connection_total_handles;


	static thread_local std::weak_ptr<pqxx::connection> handle_of_thread;
};