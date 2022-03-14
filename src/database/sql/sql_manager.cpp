#include "sql_manager.hpp"

#include <pqxx/connection>
#include <pqxx/version>

#include <Logging>
#include <Config>

sql_manager::sql_manager() :
	m_sql_address{ theConfig->sql_address },
	m_sql_port{ theConfig->sql_port},
	m_sql_database{ theConfig->sql_db },
	m_sql_user{ theConfig->sql_user },
	m_sql_pass{ theConfig->sql_pass },
	m_sql_connection_pool_size{ theConfig->sql_conn_pool_size },
	m_connection_pool{ m_sql_connection_pool_size }
{
	theLog->info("Initiating PostgreSQL Manager. Using PostgreS version: {}", PQXX_VERSION);
	theLog->info("Testing connection: '{}', table '{}', pool size: {}.", m_sql_address, m_sql_database, m_sql_connection_pool_size);

	for (std::uint8_t i = 0; i < m_sql_connection_pool_size; ++i)
		add_handle(std::make_shared<pqxx::connection>(fmt::format("host={} port={} dbname={} user={} password={}", m_sql_address, m_sql_port, m_sql_database, m_sql_user, m_sql_pass)));

	theLog->info("Database connection success!");
}

sql_handle sql_manager::acquire_handle()
{
	// Optimization idea for later: count the time a handle acquire waites for pop, and push in new sql connections accordingly
	std::shared_ptr<pqxx::connection> free_handle;
	m_connection_pool.pop(free_handle);
	return sql_handle{ *this, free_handle };
}

void sql_manager::add_handle(std::shared_ptr<pqxx::connection> conn)
{
	m_connection_pool.push(conn);
}