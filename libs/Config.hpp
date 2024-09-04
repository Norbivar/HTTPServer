#pragma once

#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <map>
#include <exception>
#include <fstream>

#include "LibSettings.hpp"

namespace Configs {
	struct config_not_found_exception : public std::runtime_error
	{
		config_not_found_exception(const std::string& str) : std::runtime_error{ str } {}
	};

	struct config_save_exception : public std::runtime_error
	{
		config_save_exception(const std::string& str) : std::runtime_error{ str } {}
	};


	class list_base
	{
		friend class config_base;
	protected:
		list_base(std::map<std::string, std::string>&& map);

		template<typename T>
		T save(const char* configname, T defaultval)
		{
			if constexpr (std::is_same<T, std::uint8_t>::value)
				settings_map[configname] = boost::lexical_cast<std::string, std::uint16_t>(defaultval);
			else if constexpr (std::is_same<T, std::int8_t>::value)
				settings_map[configname] = boost::lexical_cast<std::string, std::int16_t>(defaultval);
			else
				settings_map[configname] = boost::lexical_cast<std::string>(defaultval);

			return read<T>(configname);
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

			throw config_not_found_exception(std::string{ "Could not find config : " } + std::string{ configname });
		}

		template<typename T>
		T read(const char* configname, T defaultval)
		{
			try {
				return read<T>(configname);
			}
			catch (const config_not_found_exception& ex) {
				return save(configname, defaultval);
			}
		}

		std::map<std::string, std::string> settings_map;
	};

	class config_base
	{
	public:
		~config_base();

	protected:
		bool read_file(const char* filename, std::map<std::string, std::string>& output) const;

		void save_all();

		std::unique_ptr<Configs::list_base> m_ConfigList;
		std::unique_ptr<Configs::list_base> m_ConfigListSwap; // if by any chance, something would set a reference to a config, this would prevent UB
	};

	template<typename ConfigType>
	class config : public config_base
	{
	public:
		static config<ConfigType>& get_config()
		{
			static config<ConfigType> c{};
			return c;
		}

		config()
		{
			reload();
		}

		ConfigType* operator->() { return static_cast<ConfigType*>(m_ConfigList.get()); }

		void reload()
		{
			std::map<std::string, std::string> config_list;
			for (const auto& name : Libs::ConfigSettings::cConfigFilesToReadInOrder)
				read_file(name, config_list);

			m_ConfigListSwap = std::make_unique<ConfigType>(std::move(config_list));
			m_ConfigListSwap.swap(m_ConfigList);
		}
	};
}