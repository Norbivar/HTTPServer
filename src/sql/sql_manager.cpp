#include "sql_manager.hpp"

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/connection.h>

#include <Logging>
#include <Config>

sql_manager::sql_manager() :
	m_mysql_address{theConfig->get<Configs::mysql_address>("tcp://127.0.0.1:3306")},
	m_mysql_database{theConfig->get<Configs::mysql_db>("test")},
	m_mysql_user{theConfig->get<Configs::mysql_user>("root")},
	m_mysql_pass{theConfig->get<Configs::mysql_pass>("pass")},
	m_connection_pool_size{theConfig->get<Configs::mysql_conn_pool_size>(3)},
	m_connection_pool{m_connection_pool_size}
{
	theLog->info("Initiating MySQL Manager. Testing connection: '{}', table '{}', pool size: {}.", m_mysql_address, m_mysql_database, m_connection_pool_size);
	m_driver = get_driver_instance();
	std::shared_ptr<sql::Connection> con{ m_driver->connect(m_mysql_address, m_mysql_user, m_mysql_pass) };
	con->setSchema(m_mysql_database);

	m_connection_pool.emplace(con);
	for (std::uint8_t i = 1; i < m_connection_pool_size; ++i)
	{
		std::shared_ptr<sql::Connection> con{ m_driver->connect(m_mysql_address, m_mysql_user, m_mysql_pass) };
		con->setSchema(m_mysql_database);
		m_connection_pool.emplace(std::move(con));
	}
	theLog->info("Database connection success!");
}

sql_handle sql_manager::acquire_handle()
{
	// Optimization idea for later: count the time a handle acquire waites for pop, and push in new sql connections accordingly
	std::shared_ptr<sql::Connection> free_handle;
	m_connection_pool.pop(free_handle);
	return sql_handle{*this, free_handle};
}

void sql_manager::add_handle(std::shared_ptr<sql::Connection> conn)
{
	m_connection_pool.push(conn);
}