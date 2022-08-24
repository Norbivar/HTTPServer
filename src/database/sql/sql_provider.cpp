#include "sql_provider.hpp"

#include <Config>

std::string default_sql_provider::get_address() const
{
	return theConfig->sql_address;
}

std::uint16_t default_sql_provider::get_port() const
{
	return theConfig->sql_port;
}

std::string default_sql_provider::get_database() const
{
	return theConfig->sql_db;
}

std::string default_sql_provider::get_user() const
{
	return theConfig->sql_user;
}

std::string default_sql_provider::get_pass() const
{
	return theConfig->sql_pass;
}

std::uint8_t default_sql_provider::get_connection_desired_pool_size() const
{
	return theConfig->sql_conn_pool_size;
}

std::uint8_t default_sql_provider::get_connection_max_pool_size() const
{
	return theConfig->sql_conn_max_pool_size;
}

std::uint32_t default_sql_provider::get_connection_pool_expand_time_ms() const
{
	return theConfig->sql_conn_pool_expand_time_ms;
}