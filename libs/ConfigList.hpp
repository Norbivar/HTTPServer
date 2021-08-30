#pragma once

namespace Configs
{
	#define CREATE_CONFIG(name, temp, cstr) struct name : public Config { constexpr static const char* get_label() { return cstr; }  typedef temp type;   }
	class Config
	{
		Config() = default;
		constexpr static const char* get_label()
		{
			return "UNKNOWN";
		}
	};

	CREATE_CONFIG(log_to_file, bool,          "log_to_file");
	CREATE_CONFIG(log_level,   std::uint8_t,  "log_level");
	CREATE_CONFIG(doc_root,    std::string,   "doc_root");
	CREATE_CONFIG(cert_dir,    std::string,   "cert_dir");
	CREATE_CONFIG(bind_ip,     std::string,   "bind_ip");
	CREATE_CONFIG(port,        std::uint16_t, "port");
	CREATE_CONFIG(threads,     std::uint8_t,  "threads");

	CREATE_CONFIG(mysql_address,        std::string,   "mysql_address");
	CREATE_CONFIG(mysql_db,             std::string,   "mysql_db");
	CREATE_CONFIG(mysql_user,           std::string,   "mysql_user");
	CREATE_CONFIG(mysql_pass,           std::string,   "mysql_pass");
	CREATE_CONFIG(mysql_conn_pool_size, std::uint8_t,  "mysql_connection_pool_size");

	#undef CREATE_CONFIG
}