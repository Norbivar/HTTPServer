#pragma once

namespace Configs
{
	#define CREATE_CONFIG(name, temp, cstr) struct name : public Config { constexpr static const char* GetLabel() { return cstr; }  typedef temp type;   }
	namespace
	{
		class Config
		{
			Config() = default;
			constexpr static const char* GetLabel()
			{
				return "UNKNOWN";
			}
		};
	}

	CREATE_CONFIG(FileLoggingEnabled, bool,          "log_to_file");
	CREATE_CONFIG(LogLevel,           std::uint8_t,  "log_level");
	CREATE_CONFIG(doc_root,           std::string,   "doc_root");
	CREATE_CONFIG(bind_ip,            std::string,   "bind_ip");
	CREATE_CONFIG(port,               std::uint16_t, "port");
	CREATE_CONFIG(threads,            std::uint8_t,  "threads");

	#undef CREATE_CONFIG
}