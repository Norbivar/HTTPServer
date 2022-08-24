#pragma once

#include "Config.hpp"

namespace Configs
{
#define DEFINE_CONFIG_OPTIONAL(name, type, cstr, defaultval) const type name = read<type>(cstr, defaultval);
#define DEFINE_CONFIG(name, type, cstr) const type name = read<type>(cstr);

	class list : public Libs::config::list_base
	{
	public:
		list(std::map<std::string, std::string>&& map) : Libs::config::list_base{ std::move(map) } {}

		DEFINE_CONFIG_OPTIONAL(log_to_file, bool, "log_to_file", true);
		DEFINE_CONFIG_OPTIONAL(log_level, std::uint8_t, "log_level", 0);
		DEFINE_CONFIG_OPTIONAL(doc_root, std::string, "doc_root", "./web/");
		DEFINE_CONFIG_OPTIONAL(cert_dir, std::string, "cert_dir", "./cert/");
		DEFINE_CONFIG_OPTIONAL(bind_ip, std::string, "bind_ip", "0.0.0.0");
		DEFINE_CONFIG_OPTIONAL(port, std::uint16_t, "port", 443);
		DEFINE_CONFIG_OPTIONAL(threads, std::uint8_t, "threads", 3);

		DEFINE_CONFIG_OPTIONAL(sql_address, std::string, "sql_address", "localhost");
		DEFINE_CONFIG_OPTIONAL(sql_port, std::uint16_t, "sql_port", 5432);
		DEFINE_CONFIG_OPTIONAL(sql_db, std::string, "sql_db", "httpserver");
		DEFINE_CONFIG_OPTIONAL(sql_user, std::string, "sql_user", "postgres");
		DEFINE_CONFIG_OPTIONAL(sql_pass, std::string, "sql_pass", "asdasd");
		DEFINE_CONFIG_OPTIONAL(sql_conn_pool_size, std::uint8_t, "sql_connection_pool_size", 3);
		DEFINE_CONFIG_OPTIONAL(sql_conn_max_pool_size, std::uint8_t, "sql_connection_max_pool_size", 6);
		DEFINE_CONFIG_OPTIONAL(sql_conn_pool_expand_time_ms, std::uint32_t, "sql_connection_pool_expand_time_ms", 1000);

		DEFINE_CONFIG_OPTIONAL(session_expire_time, std::uint64_t, "session_expire_time", 60 * 60 * 60);
	};


#undef DEFINE_CONFIG_OPTIONAL
#undef DEFINE_CONFIG
}