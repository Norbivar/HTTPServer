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
	class config
	{
	public:
		static config& get_config();

		template<typename Iterable>
		config(const Iterable& file_name_list)
		{
			for (const auto& name : file_name_list)
				read_file(name);

			m_ConfigList = std::make_unique<Configs::list>(m_SettingsMap);
		}

		~config();

		Configs::list* operator->() { return m_ConfigList.get(); }

	private:
		bool read_file(const char* filename);
		void save_all();

		std::map<std::string, std::string> m_SettingsMap;
		std::unique_ptr<Configs::list> m_ConfigList;
	};
}