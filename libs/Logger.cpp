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

	const std::string& Logger::TagCache::get()
	{
		if (invalidated) // Tag was added OR removed, recalc cache
		{
			invalidated = false;
			if (tags.empty())
			{
				cached = " ";
			}
			else
			{
				cached.clear();
				for (const auto& tag : tags)
					cached += (std::string{ "[" } + tag + std::string{ "]" });
				cached += " ";
			}
		}
		return cached;
	}

	void Logger::TagCache::push(const std::string& p)
	{
		invalidated = true;
		tags.push_back(p);
	}

	void Logger::TagCache::pop()
	{
		invalidated = true;
		tags.pop_back();
	}

	Logger::Tag::Tag(const std::string& t)
	{
		Logger::GetLogger()->get_thread_local_tags().push(t);
	}
	Logger::Tag::~Tag()
	{
		Logger::GetLogger()->get_thread_local_tags().pop();
	}
}