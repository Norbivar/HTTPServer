#pragma once

#include <string>

struct sql_provider
{
	virtual std::string get_address() const = 0;
	virtual std::uint16_t get_port() const = 0;
	virtual std::string get_database() const = 0;
	virtual std::string get_user() const = 0;
	virtual std::string get_pass() const = 0;

	virtual std::uint8_t get_connection_desired_pool_size() const = 0;
	virtual std::uint8_t get_connection_max_pool_size() const = 0;

	virtual std::uint32_t get_connection_pool_expand_time_ms() const = 0;
};

struct default_sql_provider : public sql_provider
{
	std::string get_address() const override;
	std::uint16_t get_port() const override;
	std::string get_database() const override;
	std::string get_user() const override;
	std::string get_pass() const override;

	std::uint8_t get_connection_desired_pool_size() const override;
	std::uint8_t get_connection_max_pool_size() const override;
	std::uint32_t get_connection_pool_expand_time_ms() const override;
};