#include "Config.hpp"

Configs::list_base::list_base(std::map<std::string, std::string>&& map) :
	settings_map{ std::move(map) }
{}

Configs::config_base::~config_base()
{
	save_all();
}

bool Configs::config_base::read_file(const char* filename, std::map<std::string, std::string>& output) const
{
	std::ifstream input(filename);
	if (input.good())
	{
		std::string line;
		while (std::getline(input, line))
		{
			if (std::regex_match(line, Libs::ConfigSettings::cConfigValidLineRegex))
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

void Configs::config_base::save_all()
{
	if (Libs::ConfigSettings::cConfigFilesToReadInOrder.empty())
		return;

	std::ofstream output(Libs::ConfigSettings::cConfigFilesToReadInOrder.front(), std::ios::out);
	if (output.good())
	{
		for (const auto& roots : m_ConfigList->settings_map)
			output << roots.first.c_str() << "=" << roots.second.c_str() << "\n";
	}
}