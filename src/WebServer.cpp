#include "WebServer.hpp"

#include <boost/asio/ip/address.hpp>
#include <boost/asio/signal_set.hpp>

#include <Logging>
#include "base/listener.hpp"
#include "base/websocket_session.hpp"
#include "base/http_session.hpp"

#include "../cert/server_certificate.hpp"

webserver::webserver(const boost::asio::ip::address& address, const uint16_t port, const std::shared_ptr<std::string>& doc_root, std::uint8_t numthreads) :
	m_address{address},
	m_port{port},
	m_doc_root{doc_root},
	m_desired_thread_number{numthreads},
	m_ioc{numthreads},
	m_ctx{boost::asio::ssl::context::tlsv12}
{
	assert(numthreads != 0);
	m_threads.reserve(numthreads - 1);
}

void webserver::bootstrap()
{
	theLog->info("Bootstrapping WebServer...");
	// This holds the self-signed certificate used by the server
	load_server_certificate(m_ctx);
	theLog->info("->	Certificates loaded.");
	m_routing_table.register_all();
	theLog->info("->	Path mapping set up.");
	m_routing_table.print_stats();
}

int webserver::run()
{
	boost::beast::error_code ec;

	theLog->info("Preparing WebServer...");

	std::make_shared<listener>(m_ioc, m_ctx, boost::asio::ip::tcp::endpoint{ m_address, m_port }, *this)->run();

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	boost::asio::signal_set signals(m_ioc, SIGINT, SIGTERM);
	signals.async_wait([&](const boost::beast::error_code&, int) {
		m_ioc.stop();
	});

	for (auto i = 1; i < m_desired_thread_number; ++i)
		m_threads.emplace_back([this] { m_ioc.run(); });

	theLog->info("WebServer runnning on port {}", m_port);

	m_ioc.run();

	// Block until all the threads exit
	for (auto& t : m_threads)
		t.join();

	theLog->info("WebServer stopped.");

	return EXIT_SUCCESS;
}