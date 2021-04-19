#include <fstream>
#include <iostream>

#include "Logger.hpp"
namespace Libs
{
	Logger* Logger::GetLogger()
	{
		try
		{
			static Logger s;
			return &s;
		}
		catch (const spdlog::spdlog_ex& ex)
		{
			std::cout << ex.what() << std::endl;
			// exception will probably be missing txt files. I haven't found any clue about the logger, so I do it manually
			std::ifstream finServ(LoggerSettings::cLogFile);
			if (!finServ)
			{
				std::ofstream outfile(LoggerSettings::cLogFile);
			}
			static Logger s;
			return &s;
		}
	}
}