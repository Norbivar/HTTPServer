#include "WebServer.hpp"

#include <boost/asio/ip/address.hpp>
#include <boost/asio/signal_set.hpp>

#include <Logging>
#include "base/listener.hpp"

#include "../cert/server_certificate.hpp"

WebServer::WebServer(const boost::asio::ip::address& address, const uint16_t port, const std::shared_ptr<std::string>& doc_root, std::uint8_t numthreads) :
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

void WebServer::Booststrap()
{
	// This holds the self-signed certificate used by the server
	load_server_certificate(m_ctx);
}

int WebServer::Run()
{
	std::make_shared<listener>(m_ioc, m_ctx, boost::asio::ip::tcp::endpoint{ m_address, m_port }, m_doc_root)->run();

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	boost::asio::signal_set signals(m_ioc, SIGINT, SIGTERM);
	signals.async_wait([&](boost::beast::error_code const&, int) {
		m_ioc.stop();
	});

	for (auto i = 1; i < m_desired_thread_number; ++i)
		m_threads.emplace_back([this] { m_ioc.run(); });

	m_ioc.run();

	// Block until all the threads exit
	for (auto& t : m_threads)
		t.join();

	return EXIT_SUCCESS;
}