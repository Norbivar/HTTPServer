#include <memory>
#include <iostream>

#include <boost/asio/ip/address.hpp>

#include "WebServer.hpp"

int main(int argc, char* argv[])
{
	std::cout <<"asd";
	std::cout.flush();
	// Check command line arguments.
	if (argc != 2)
	{
		std::cerr << "Usage: server <doc_root>\n";
		return EXIT_FAILURE;
	}
	const auto address = boost::asio::ip::make_address("0.0.0.0");
	const auto port = 80;
	const auto doc_root = std::make_shared<std::string>(argv[1]);
	const auto threads = 3;

	WebServer server{ address, port, doc_root, threads };
	server.Booststrap();
	return server.Run();
}