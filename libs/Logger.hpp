#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "LibSettings.hpp"

#include "Config"

namespace Libs
{
	class Logger
	{
		struct TagCache
		{
			const std::string& get();
			void push(const std::string& p);
			void pop();
		private:
			bool invalidated = false;
			std::string cached{" "};

			std::vector<std::string> tags;
		};

	public:
		struct Tag
		{
			Tag(const std::string& t);
			~Tag();
		};
		friend struct Tag;

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
			if (!setTo)
				spdlog::set_pattern("[%H:%M:%S]{%l}%v");
			else
				spdlog::set_pattern("%v");
		}

		template<typename Arg1, typename... Args>
		inline void trace(const std::string& fmt, const Arg1& arg1, const Args& ... args)
		{
			std::string tagged_message = get_thread_local_tags().get() + fmt;
			if (RotatedTxtLogger)
				RotatedTxtLogger->trace(tagged_message.c_str(), arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->trace(tagged_message.c_str(), arg1, args...);
		}
		template<typename T>
		inline void trace(const T& msg)
		{
			std::string tagged_message = get_thread_local_tags().get() + msg;
			if (RotatedTxtLogger)
				RotatedTxtLogger->trace(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->trace(tagged_message);
		}

		template<typename Arg1, typename... Args>
		inline void debug(const std::string& fmt, const Arg1& arg1, const Args& ... args)
		{
			std::string tagged_message = get_thread_local_tags().get() + fmt;
			if (RotatedTxtLogger)
				RotatedTxtLogger->debug(tagged_message.c_str(), arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->debug(tagged_message.c_str(), arg1, args...);
		}
		template<typename T>
		inline void debug(const T& msg)
		{
			std::string tagged_message = get_thread_local_tags().get() + msg;
			if (RotatedTxtLogger)
				RotatedTxtLogger->debug(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->debug(tagged_message);
		}

		template<typename Arg1, typename... Args>
		inline void info(const std::string& fmt, const Arg1& arg1, const Args &... args)
		{
			std::string tagged_message = get_thread_local_tags().get() + fmt;
			if (RotatedTxtLogger)
				RotatedTxtLogger->info(tagged_message.c_str(), arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->info(tagged_message.c_str(), arg1, args...);
		}
		template<typename T>
		inline void info(const T& msg)
		{
			std::string tagged_message = get_thread_local_tags().get() + msg;
			if (RotatedTxtLogger)
				RotatedTxtLogger->info(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->info(tagged_message);
		}

		template<typename Arg1, typename... Args>
		inline void warn(const std::string& fmt, const Arg1& arg1, const Args &... args)
		{
			std::string tagged_message = get_thread_local_tags().get() + fmt;
			if (RotatedTxtLogger)
				RotatedTxtLogger->warn(tagged_message.c_str(), arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->warn(tagged_message.c_str(), arg1, args...);
		}
		template<typename T>
		inline void warn(const T& msg)
		{
			std::string tagged_message = get_thread_local_tags().get() + msg;
			if (RotatedTxtLogger)
				RotatedTxtLogger->warn(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->warn(tagged_message);
		}

		inline void warn(const std::exception& err)
		{
			std::string tagged_message = fmt::format("{}{}", get_thread_local_tags().get(), err.what());
			if (RotatedTxtLogger)
				RotatedTxtLogger->warn(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->warn(tagged_message);
		}

		template<typename Arg1, typename... Args>
		inline void error(const std::string& fmt, const Arg1& arg1, const Args &... args)
		{
			std::string tagged_message = get_thread_local_tags().get() + fmt;
			if (RotatedTxtLogger)
				RotatedTxtLogger->error(tagged_message.c_str(), arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->error(tagged_message.c_str(), arg1, args...);
		}
		template<typename T>
		inline void error(const T& msg)
		{
			std::string tagged_message = get_thread_local_tags().get() + msg;
			if (RotatedTxtLogger)
				RotatedTxtLogger->error(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->error(tagged_message);
		}

		inline void error(const std::exception& err)
		{
			std::string tagged_message = fmt::format("{}{}", get_thread_local_tags().get(), err.what());
			if (RotatedTxtLogger)
				RotatedTxtLogger->error(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->error(tagged_message);
		}

		template<typename Arg1, typename... Args>
		inline void critical(const std::string& fmt, const Arg1& arg1, const Args& ... args)
		{
			std::string tagged_message = get_thread_local_tags().get() + fmt;
			if (RotatedTxtLogger)
				RotatedTxtLogger->critical(tagged_message.c_str(), arg1, args...);
			if (ConsoleLogger)
				ConsoleLogger->critical(tagged_message.c_str(), arg1, args...);
		}
		template<typename T>
		inline void critical(const T& msg)
		{
			std::string tagged_message = get_thread_local_tags().get() + msg;
			if (RotatedTxtLogger)
				RotatedTxtLogger->critical(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->critical(tagged_message);
		}

		inline void critical(const std::exception& err)
		{
			std::string tagged_message = fmt::format("{}{}", get_thread_local_tags().get(), err.what());
			if (RotatedTxtLogger)
				RotatedTxtLogger->critical(tagged_message);
			if (ConsoleLogger)
				ConsoleLogger->critical(tagged_message);
		}

	private:
		std::shared_ptr<spdlog::logger> RotatedTxtLogger;
		std::shared_ptr<spdlog::logger> ConsoleLogger;

		TagCache& get_thread_local_tags() const { static thread_local TagCache tags; return tags; }
	};
}