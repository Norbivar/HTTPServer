#pragma once

#include <Config.hpp>

namespace Configs
{
#define DEFINE_CONFIG_OPTIONAL(name, type, cstr, defaultval) const type name = read<type>(cstr, defaultval);
#define DEFINE_CONFIG(name, type, cstr) const type name = read<type>(cstr);

	class load_balancer_config_list : public list_base
	{
	public:
		load_balancer_config_list(std::map<std::string, std::string>&& map) : list_base{ std::move(map) } {}

		DEFINE_CONFIG_OPTIONAL(log_to_file, bool, "log_to_file", true);
		DEFINE_CONFIG_OPTIONAL(log_level, std::uint8_t, "log_level", 0);

		// Directory of the HTTPS cert
		DEFINE_CONFIG_OPTIONAL(cert_dir, std::string, "cert_dir", "cert/");
		DEFINE_CONFIG_OPTIONAL(bind_ip, std::string, "bind_ip", "0.0.0.0");
		DEFINE_CONFIG_OPTIONAL(port, std::uint16_t, "port", 443);
		DEFINE_CONFIG_OPTIONAL(threads, std::uint8_t, "threads", 3);
	};

#undef DEFINE_CONFIG_OPTIONAL
#undef DEFINE_CONFIG
}