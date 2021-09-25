#include <fstream>
#include "Config.hpp"

namespace Libs
{
	config& config::get_config()
	{
		static config c{ConfigSettings::cConfigFilesToReadInOrder};
		return c;
	}

	config::~config()
	{
		save_all();
	}

	void config::save_all()
	{
		std::ofstream output(*ConfigSettings::cConfigFilesToReadInOrder.begin(), std::ios::out);
		if (output.good())
		{
			for (const auto& roots : m_SettingsMap)
			{
				output << roots.first.c_str() << "=" << roots.second.c_str() << "\n";
			}
		}
	}

	bool config::read_file(const char* filename)
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

					m_SettingsMap.emplace(key, valueText);
				}
			}
			return true;
		}
		else return false;
	}
}