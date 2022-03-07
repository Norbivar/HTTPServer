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
		DEFINE_CONFIG_OPTIONAL(cert_dir, std::string, "cert_dir", "./cert");
		DEFINE_CONFIG_OPTIONAL(bind_ip, std::string, "bind_ip", "0.0.0.0");
		DEFINE_CONFIG_OPTIONAL(port, std::uint16_t, "port", 443);
		DEFINE_CONFIG_OPTIONAL(threads, std::uint8_t, "threads", 3);

		DEFINE_CONFIG_OPTIONAL(mysql_address, std::string, "mysql_address", "tcp://127.0.0.1:3306");
		DEFINE_CONFIG_OPTIONAL(mysql_db, std::string, "mysql_db", "test");
		DEFINE_CONFIG_OPTIONAL(mysql_user, std::string, "mysql_user", "root");
		DEFINE_CONFIG_OPTIONAL(mysql_pass, std::string, "mysql_pass", "qwertzui");
		DEFINE_CONFIG_OPTIONAL(mysql_conn_pool_size, std::uint8_t, "mysql_connection_pool_size", 3);

		DEFINE_CONFIG_OPTIONAL(session_expire_time, std::uint64_t, "session_expire_time", 60 * 60 * 60);
	};


#undef DEFINE_CONFIG_OPTIONAL
#undef DEFINE_CONFIG
}