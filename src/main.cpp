#include <memory>
#include <iostream>

#include <boost/asio/ip/address.hpp>

#include <Logging>
#include <Config>
#include "webserver.hpp"

int main(int /*argc*/, char* /*argv*/[])
{
	theLog->info("Startup in progress");
	try
	{
		theLog->set_file_logging(theConfig->log_to_file);
		theLog->set_log_level(static_cast<spdlog::level::level_enum>(theConfig->log_level));

		auto& server = webserver::instance();
		server.bootstrap();
		return server.run();
	}
	catch (const std::exception& e)
	{
		theLog->critical("Unhandled exception encountered: {}", e.what());
		return -1;
	}
}