#include <memory>
#include <iostream>

#include <boost/asio/ip/address.hpp>

#include <Logging>
#include <Config>
#include "WebServer.hpp"

int main(int /*argc*/, char* /*argv*/[])
{
	theLog->info("Startup in progress");
	// Mandatory:
	const auto doc_root = std::make_shared<std::string>(theConfig->Get<Configs::doc_root>());

	// Optional:
	const auto address = boost::asio::ip::make_address(theConfig->Get<Configs::bind_ip>("0.0.0.0"));
	const auto port    = theConfig->Get<Configs::port>(80);
	const auto threads = theConfig->Get<Configs::threads>(3);

	WebServer server{ address, port, doc_root, threads };
	server.Booststrap();
	return server.Run();
}