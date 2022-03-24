#pragma once

#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <map>
#include <exception>

#include "LibSettings.hpp"

namespace Configs { class list; }

namespace Libs
{
	class config
	{
	public:
		class list_base
		{
			friend class config;
		protected:
			list_base(std::map<std::string, std::string>&& map);

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

				throw std::runtime_error(std::string{ "Could not find config : " + std::string(configname) });
			}

			std::map<std::string, std::string> settings_map;
		};

		static config& get_config();

		config();
		~config();

		Configs::list* operator->() { return m_ConfigList.get(); }

		void reload();
	private:
		bool read_file(const char* filename, std::map<std::string, std::string>& output) const;
		void save_all();

		std::unique_ptr<Configs::list> m_ConfigList;
		std::unique_ptr<Configs::list> m_ConfigListSwap; // if by any chance, something would set a reference to a config, this would prevent UB
	};

}