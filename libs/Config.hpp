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

		config();
		~config();

		Configs::list* operator->() { return m_ConfigList.get(); }

		void reload();
	private:
		bool read_file(const char* filename);
		void save_all();

		std::map<std::string, std::string> m_SettingsMap;
		std::unique_ptr<Configs::list> m_ConfigList;
		std::unique_ptr<Configs::list> m_ConfigListSwap; // if by any chance, something would set a reference to a config, this would prevent UB
	};
}