#pragma once

#include <boost/lexical_cast.hpp>
#include <boost/any.hpp> // TODO: is this needed?
#include <boost/optional/optional.hpp>
#include <map>
#include <exception>

#include "LibSettings.hpp"
#include "ConfigList.hpp"

namespace Libs
{
	struct ConfigOrFileNotFoundException : public std::exception
	{
		ConfigOrFileNotFoundException(const std::string& msg) : msg(msg) { }
		const char* what() const throw ()
		{
			return msg.c_str();
		}
		
		const std::string msg;
	};

	class config
	{
	public:
		static config* get_config();

		template<typename Iterable>
		config(const Iterable& file_name_list)
		{
			for (const auto& name : file_name_list)
				read_file(name);
		}

		~config();

		template<typename config>
		boost::optional<typename config::type> get_optional()
		{
			using T = typename config::type;

			const auto& configname = config::get_label();
			const auto& node = m_SettingsRootMap.find(configname);
			if (node != m_SettingsRootMap.end())
			{
				if constexpr (std::is_same<T, std::uint8_t>::value)
					return static_cast<T>(boost::lexical_cast<std::uint16_t>(node->second));
				else if constexpr (std::is_same<T, std::int8_t>::value)
					return static_cast<T>(boost::lexical_cast<std::int16_t>(node->second));
				else
					return boost::lexical_cast<T>(node->second);
			}
			else
				return boost::none;
		}

		// Returns the first match of "configname" or the default value if not found any.
		// Wrong types will cause a boost::bad_cast exception (boost::any-based).
		template<typename config>
		typename config::type get(const typename config::type& defaultval)
		{
			const auto conf = get_optional<config>();
			if (conf)
				return *conf;
			else
			{
				set<config>(defaultval); // this will make sure that not found configs at first run will get printed out to .ini. TODO: think this through
				return defaultval;
			}
		}

		template<typename config>
		typename config::type get()
		{
			const auto conf = get_optional<config>();
			if (conf)
				return *conf;
			else
				throw ConfigOrFileNotFoundException(std::string("CONFIG: Could not find:" + std::string(config::get_label())));
		}

		template<typename config>
		void set(const typename config::type& setto)
		{
			using T = typename config::type;

			const auto& configname = config::get_label();
			if constexpr (std::is_same<T, std::uint8_t>::value)
				m_SettingsRootMap[configname] = boost::lexical_cast<std::string, std::uint16_t>(setto);
			else if constexpr (std::is_same<T, std::int8_t>::value)
				m_SettingsRootMap[configname] = boost::lexical_cast<std::string, std::int16_t>(setto);
			else
				m_SettingsRootMap[configname] = boost::lexical_cast<std::string>(setto);
		}

		void save_all_to_file(const char* filename);
		void save_all();

	private:
		bool read_file(const char* filename);

		std::map<std::string, std::string> m_SettingsRootMap;
	};
}