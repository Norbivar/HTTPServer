#include "sql_manager.hpp"

#include <chrono>

sql_manager::sql_manager(const std::string& name, const sql_provider& provider)
	:
	m_name{ name },
	m_sql_address{ provider.get_address() },
	m_sql_port{ provider.get_port() },
	m_sql_database{ provider.get_database() },
	m_sql_user{ provider.get_user() },
	m_sql_pass{ provider.get_pass() },
	m_sql_connection_desired_pool_size{ provider.get_connection_desired_pool_size() },
	m_sql_connection_max_pool_size{ provider.get_connection_max_pool_size() },
	m_sql_connection_pool_expand_time{ provider.get_connection_pool_expand_time_ms() },
	m_sql_connection_total_handles { 0 }
{
	m_connection_pool.reserve(m_sql_connection_max_pool_size);

	theLog->info("Initiating SQL Connection '{}' : address '{}' table '{}', pool size: {}, max pool size: {}", m_name, m_sql_address, m_sql_database, m_sql_connection_desired_pool_size, m_sql_connection_max_pool_size);

	for (std::uint8_t i = 0; i < m_sql_connection_desired_pool_size; ++i)
		push_new_handle();

	theLog->info("Database connection success!");
}

void sql_manager::push_new_handle()
{
	m_connection_pool.push_back(std::make_shared<pqxx::connection>(fmt::format("host={} port={} dbname={} user={} password={}", m_sql_address, m_sql_port, m_sql_database, m_sql_user, m_sql_pass)));
	++m_sql_connection_total_handles;
}

thread_local std::weak_ptr<pqxx::connection> sql_manager::handle_of_thread{}; // since it is threadlocal, it should not need mutex access

sql_handle sql_manager::acquire_handle() // threadsafe
{
	if (handle_of_thread.use_count() > 0) // if this thread already holds a handle -> just return that
	{
		return sql_handle{ *this, handle_of_thread.lock() };
	}
	else
	{
		std::unique_lock lock{ m_connection_pool_mutex };

		if (m_connection_pool.empty())
		{
			while (!m_connection_pool_push_event.wait_for(lock, std::chrono::milliseconds{ m_sql_connection_pool_expand_time }, [&] { return !m_connection_pool.empty(); })) // managed to receive an event
			{
				if (m_sql_connection_total_handles < m_sql_connection_max_pool_size)
					push_new_handle(); // Does not need the notify, because this thread can pick it up right away
			}
		}

		std::shared_ptr<pqxx::connection> free_handle = m_connection_pool.back();
		m_connection_pool.pop_back();

		handle_of_thread = free_handle;
		return sql_handle{ *this, free_handle };
	}
}

void sql_manager::add_handle(const std::shared_ptr<pqxx::connection> conn) // threadsafe
{
	std::unique_lock lock{ m_connection_pool_mutex };

	handle_of_thread.reset();
	m_connection_pool.push_back(conn);

	m_connection_pool_push_event.notify_one();
}