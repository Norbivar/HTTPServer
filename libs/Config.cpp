#include <fstream>
#include "Config.hpp"

namespace Libs
{
	Config* Config::GetConfig()
	{
		static Config c{ConfigSettings::cConfigFilesToReadInOrder};
		return &c;
	}

	Config::~Config()
	{
		SaveAll();
	}

	void Config::SaveAllConfigTo(const char* filename)
	{
		std::ofstream output(filename, std::ios::out);
		if (output.good())
		{
			for (const auto& roots : m_SettingsRootMap)
			{
				output << roots.first.c_str() << "=" << roots.second.c_str() << "\n";
			}
		}
	}
	void Config::SaveAll()
	{
		if (!ConfigSettings::cConfigFilesToReadInOrder.empty())
		{
			const char* filename = *ConfigSettings::cConfigFilesToReadInOrder.begin();
			SaveAllConfigTo(filename);
		}
	}

	bool Config::ReadFile(const char* filename)
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

					if (m_SettingsRootMap.find(key) != m_SettingsRootMap.end())
					{
						continue;
					}

					m_SettingsRootMap.emplace(key, valueText);
				}
			}
			return true;
		}
		else return false;
	}
}