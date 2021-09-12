#pragma once

namespace Configs
{
	struct optional_config_tag {};

	#define CREATE_CONFIG(name, temp, cstr)  \
	struct name        \
	{                                        \
		constexpr static const char* get_label() { return cstr; }\
		typedef temp type;  \
	};

#define CREATE_OPTIONAL_CONFIG(name, temp, cstr, defaultval)  \
	struct name : optional_config_tag         \
	{                                        \
	 constexpr static const char* get_label() { return cstr; }\
			typedef temp type;  \
		static type get_default() { return defaultval; } \
	};


	CREATE_OPTIONAL_CONFIG(log_to_file, bool,          "log_to_file", true);
	CREATE_OPTIONAL_CONFIG(log_level,   std::uint8_t,  "log_level",   0);
	CREATE_OPTIONAL_CONFIG(doc_root,    std::string,   "doc_root",    "./web/");
	CREATE_OPTIONAL_CONFIG(cert_dir,    std::string,   "cert_dir",    "./cert");
	CREATE_OPTIONAL_CONFIG(bind_ip,     std::string,   "bind_ip",     "0.0.0.0");
	CREATE_OPTIONAL_CONFIG(port,        std::uint16_t, "port",        443);
	CREATE_OPTIONAL_CONFIG(threads,     std::uint8_t,  "threads",     3);

	CREATE_OPTIONAL_CONFIG(mysql_address,        std::string,   "mysql_address",             "tcp://127.0.0.1:3306");
	CREATE_OPTIONAL_CONFIG(mysql_db,             std::string,   "mysql_db",                  "test");
	CREATE_OPTIONAL_CONFIG(mysql_user,           std::string,   "mysql_user",                "root");
	CREATE_OPTIONAL_CONFIG(mysql_pass,           std::string,   "mysql_pass",                "pass");
	CREATE_OPTIONAL_CONFIG(mysql_conn_pool_size, std::uint8_t,  "mysql_connection_pool_size", 3);

	#undef CREATE_CONFIG
}