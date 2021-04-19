#pragma once
namespace Libs
{
	namespace Configs
	{
		#define CREATE_CONFIG(name, temp, cstr) struct name : public Config { constexpr static const char* getValue() { return cstr; }  typedef temp type;   }
		

		class Config
		{
			Config() = default;
			constexpr static const char* getValue()
			{
				return "UNKOWN";
			}
		};

		CREATE_CONFIG(FileLoggingEnabled, bool,  "log_to_file");
		CREATE_CONFIG(LogLevel,           int,   "log_level");

		#undef CREATE_CONFIG
	}
}