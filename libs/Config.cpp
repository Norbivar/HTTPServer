#include <fstream>
#include "Config.hpp"
#include "ConfigList.hpp"

namespace Libs
{
	config::list_base::list_base(std::map<std::string, std::string>&& map) :
		settings_map{ std::move(map) }
	{}

	config& config::get_config()
	{
		static config c{};
		return c;
	}

	config::config()
	{
		reload();
	}

	void config::reload()
	{
		std::map<std::string, std::string> config_list;
		for (const auto& name : Libs::ConfigSettings::cConfigFilesToReadInOrder)
			read_file(name, config_list);

		m_ConfigListSwap = std::make_unique<Configs::list>(std::move(config_list));
		m_ConfigListSwap.swap(m_ConfigList);
	}

	config::~config()
	{
		save_all(); // A config should not really require saving
	}

	void config::save_all()
	{
		if (ConfigSettings::cConfigFilesToReadInOrder.empty())
			return;

		std::ofstream output(ConfigSettings::cConfigFilesToReadInOrder.front(), std::ios::out);
		if (output.good())
		{
			for (const auto& roots : m_ConfigList->settings_map)
				output << roots.first.c_str() << "=" << roots.second.c_str() << "\n";
		}
	}

	bool config::read_file(const char* filename, std::map<std::string, std::string>& output) const
	{
		std::ifstream input(filename);
		if (input.good())
		{
			std::string line;
			while (std::getline(input, line))
			{
				if (std::regex_match(line, ConfigSettings::cConfigValidLineRegex))
				{
					const auto eqpos = line.find_first_of('=');
					const std::string key = line.substr(0, eqpos);
					const std::string valueText = line.substr(eqpos + 1);

					output[key] = valueText;
				}
			}
			return true;
		}
		return false;
	}
}