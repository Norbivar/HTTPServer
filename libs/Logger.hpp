#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "LibSettings.hpp"

#include "Config"

namespace Libs
{
	class Logger
	{
	public:
		static Logger* GetLogger();
		Logger()
		{
			ConsoleLogger = spdlog::stdout_color_mt("console");

			spdlog::set_async_mode(8192);
			spdlog::set_level(spdlog::level::level_enum::info);
			spdlog::flush_on(spdlog::level::err);

			toggleLogOnlyText(false);
			spdlog::drop_all();
		}
		~Logger() 
		{ 
			if (RotatedTxtLogger)
				RotatedTxtLogger->flush();
		}

		void set_log_level(spdlog::level::level_enum level)
		{
			spdlog::set_level(level);
		}

		void set_file_logging(bool enable)
		{
			if (enable && !RotatedTxtLogger)
				RotatedTxtLogger = spdlog::rotating_logger_mt("txtlogger", LoggerSettings::cLogFile, 1024 * 1024 * 20, 3);
			else if (!enable && RotatedTxtLogger)
				RotatedTxtLogger.reset();
		}

		inline void toggleLogOnlyText(bool setTo)
		{
			if(!setTo)
				spdlog::set_pattern("[%H:%M:%S] %l : %v");
			else
				spdlog::set_pattern("%v");
		}

		template<typename Arg1, typename... Args>
		inline void trace(const char* fmt, const Arg1& arg1, const Args& ... args)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->trace(fmt, arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->trace(fmt, arg1, args...);
		}
		template<typename T>
		inline void trace(const T& msg)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->trace(msg);
			if (ConsoleLogger)
				ConsoleLogger->trace(msg);
		}

		template<typename Arg1, typename... Args>
		inline void debug(const char* fmt, const Arg1& arg1, const Args& ... args)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->debug(fmt, arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->debug(fmt, arg1, args...);
		}
		template<typename T>
		inline void debug(const T& msg)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->debug(msg);
			if (ConsoleLogger)
				ConsoleLogger->debug(msg);
		}

		template<typename Arg1, typename... Args>
		inline void info(const char *fmt, const Arg1 &arg1, const Args &... args)
		{
			if(RotatedTxtLogger)
				RotatedTxtLogger->info(fmt, arg1, args...);
			if(ConsoleLogger)
				ConsoleLogger->info(fmt, arg1, args...);
		}
		template<typename T>
		inline void info(const T& msg)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->info(msg);
			if (ConsoleLogger)
				ConsoleLogger->info(msg);
		}

		template<typename Arg1, typename... Args>
		inline void warn(const char *fmt, const Arg1 &arg1, const Args &... args)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->warn(fmt, arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->warn(fmt, arg1, args...);
		}
		template<typename T>
		inline void warn(const T& msg)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->warn(msg);
			if (ConsoleLogger)
				ConsoleLogger->warn(msg);
		}

		template<typename Arg1, typename... Args>
		inline void error(const char *fmt, const Arg1 &arg1, const Args &... args)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->error(fmt, arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->error(fmt, arg1, args...);
		}
		template<typename T>
		inline void error(const T& msg)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->error(msg);
			if (ConsoleLogger)
				ConsoleLogger->error(msg);
		}

		template<typename Arg1, typename... Args>
		inline void critical(const char* fmt, const Arg1& arg1, const Args& ... args)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->critical(fmt, arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->critical(fmt, arg1, args...);
		}
		template<typename T>
		inline void critical(const T& msg)
		{
			if (RotatedTxtLogger)
				RotatedTxtLogger->critical(msg);
			if (ConsoleLogger)
				ConsoleLogger->critical(msg);
		}

	private:
		std::shared_ptr<spdlog::logger> RotatedTxtLogger;
		std::shared_ptr<spdlog::logger> ConsoleLogger;
	};
}