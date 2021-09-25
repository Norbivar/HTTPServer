#pragma once

namespace Configs
{
#define DEFINE_CONFIG_OPTIONAL(name, type, cstr, defaultval) const type name = read<type>(cstr, defaultval);
#define DEFINE_CONFIG(name, type, cstr)  const type name = read<type>(cstr);

	struct list
	{
	private:
		std::map<std::string, std::string>& settings_map;

	public:
		list(std::map<std::string, std::string>& map) : settings_map{ map } {}

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

	private:
		template<typename T>
		T read(const char* configname, T defaultval)
		{
			const auto& node = settings_map.find(configname);
			if (node != settings_map.end())
			{
				if constexpr (std::is_same<T, std::uint8_t>::value)
					return static_cast<T>(boost::lexical_cast<std::uint16_t>(node->second));
				else if constexpr (std::is_same<T, std::int8_t>::value)
					return static_cast<T>(boost::lexical_cast<std::int16_t>(node->second));
				else
					return boost::lexical_cast<T>(node->second);
			}
			else
			{
				if constexpr (std::is_same<T, std::uint8_t>::value)
					settings_map[configname] = boost::lexical_cast<std::string, std::uint16_t>(defaultval);
				else if constexpr (std::is_same<T, std::int8_t>::value)
					settings_map[configname] = boost::lexical_cast<std::string, std::int16_t>(defaultval);
				else
					settings_map[configname] = boost::lexical_cast<std::string>(defaultval);

				return defaultval;
			}
		}

		template<typename T>
		T read(const char* configname)
		{
			const auto& node = settings_map.find(configname);
			if (node != settings_map.end())
			{
				if constexpr (std::is_same<T, std::uint8_t>::value)
					return static_cast<T>(boost::lexical_cast<std::uint16_t>(node->second));
				else if constexpr (std::is_same<T, std::int8_t>::value)
					return static_cast<T>(boost::lexical_cast<std::int16_t>(node->second));
				else
					return boost::lexical_cast<T>(node->second);
			}

			throw ConfigOrFileNotFoundException(std::string("CONFIG: Could not find:" + std::string(config::get_label())));
		}

	};


#undef DEFINE_CONFIG_OPTIONAL
#undef DEFINE_CONFIG
}