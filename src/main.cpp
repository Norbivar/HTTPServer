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
		auto& server = webserver::instance();
		server.bootstrap();
		return server.run();
	}
	catch (const std::exception& e)
	{
		theLog->critical("Exception encountered: {}", e.what());
		return -1;
	}
}