#include <memory>
#include <iostream>

#include <boost/asio/ip/address.hpp>

#include <Logging>
#include <Config>
#include "webserver.hpp"

int main(int /*argc*/, char* /*argv*/[])
{
	theLog->info("Startup in progress");

	auto& theServer = webserver::instance();
	theServer.bootstrap();
	return theServer.run();
}